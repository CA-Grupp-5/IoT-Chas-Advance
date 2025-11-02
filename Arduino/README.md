# Sensor Package Device

This repository contains the firmware for the Sensor Package device(Arduino Uno R4 WiFi), designed to read temperature and humidity data from a DHT11 sensor, establish a TCP connection over WiFi, and transmit the sensor data to a Control Unit/Server (ESP32 S3 Zero).

The application logic is driven by a Finite State Machine for predicatble operation, connection management, error handling, and timing.

## High Level Design

The application is structures around a State Machine implemented in the `SensorPackage` library. This design separates the core application logic from the Arduino's `setup()` and `loop()` functions.

### Data and drivers

-   Global Device State (`SensorPackage` struct): Holds all mutable application data, including the latest sensor readings (`PackageData`), the current state(`State`), timing variables(`current_time`,`last_sent`, `response_wait_start`), network credentials (`ssid`, `pass`), and the raw data payload buffer (`raw_payload`)
-   External Drivers (`SensorDrivers` struct): Holds references to allnecessary external library instances (`DHT`, `WiFiClient`, `IPAddress`). This allows the the core logic to the be test able and decouples it from the global driver instantiation in `main.cpp`

### Application Flow

The Arduino's `loop()` consists of a single, continuous call to the state machine's execution function:

```
void loop()
{
    runSensorPackage(&package, &currentDrivers);
}
```

The `runSensorPackage` function uses the current value of the `State` enum (`package->state`) as an index into a loopup table (`stateTable`) of function pointers. This executes the corresponding state function (`StateFunction`) on every loop iteration, driving the application forward.

### Library Dependencies

-   Core: `<Arduino.h>`
-   WiFi Connectivity: `<WiFiS3.h>`
-   Sensor: `<DHT.h>` and `<Adafruit_Sensor.h>`
-   Custom libraries: `"States.h"` (Defines the State Machine enum) and `<SensorPackage.h>` (Defines core structures and function prototypes)

#### Memory and efficiency Considerations

-   Non-blocking timing: The State machine approach is efficient because the `loop()` function execution quickly, transitioning between state without long, blocking delays.
-   String/Buffer Management: The raw payload is pre-formatted into a fixed-sized buffer (`raw_payload[PAYLOAD_BUFFER_SIZE]`) using `snprintf()`. This avoids dynamic memory allocation and fragmentation issues.

## Code and Logic Implementation Details

### State Machine and States

The core logic is within the defined states, managing the device's operation flow:

| State                | Purpose                       | Next State                                                                                                                                                                                                                                                                                                                                    |
| -------------------- | ----------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| STATE_INIT           | Initialization                | Initializes the DHT driver (`dht.begin()`), sets the device ID, performs an initial sensor read, and transitions to STATE_CONNECT_WIFI                                                                                                                                                                                                        |
| STATE_CONNECT_WIFI   | WiFi Connection               | Attempts to connect to the configured WiFi network (`SECRET_SSID`, `SECRET_PASSWORD`). Includes a 30-second timeout for the initial connection attempt. If the connection fails repeatedly, it returns to retry. If the WiFi module is missing (`WL_NO_MODULE`), it transitions to STATE_ERROR. On success, it moves to STATE_CONNECT_SERVER. |
| STATE_CONNECT_SERVER | TCP Connection                | Attempts to establish a TCP connection with the Control Unit/Server using the configured `SERVER_IP` and `SERVER_PORT`. On success, transitions to STATE_SEND. On failure, it transitions back to STATE_CONNECT_WIFI to ensure the network is stable before retrying.                                                                         |
| STATE_WAIT_INTERVAL  | Transmission Timing           | Implements a non-blocking wait using `millis()` to enforce a minimum interval of 60000 ms (1 minute) (`TRANSMISSION_INTERVAL_MS`) between successful data transmissions. After the interval, it transitions to STATE_SEND.                                                                                                                    |
| STATE_SEND           | Data Capture and Send         | Calls `captureSensorData` to read, format, and prepare the payload. It then sends the `raw_payload` string over the established TCP connection using `drivers->client.print()`. Sets a timestamp (`response_wait_start`) and moves to STATE_WAIT_RESPONSE.                                                                                    |
| STATE_WAIT_RESPONSE  | Response handling and timeout | Waits for available data (response) from the server. If data is available, it calls `handleResponse` (reads, prints, and closes connection). If the wait time exceeds 3000 ms (3 seconds) (`REPLY_TIMEOUT_MS`) without a response, a timeout occurs, the connection is closed, and it transitions to STATE_WAIT_INTERVAL for the next cycle.  |
| STATE_ERROR          | Critical Error                | A terminal state triggered by persistent sensor errors or a critical hardware/module failure. It halts normal operation and prints an error message.                                                                                                                                                                                          |

### DHT11 Sensor Reading

-   Driver Library: The DHT driver is an instance of the standard `DHT` library, configured for pin 7 (`DHTPIN`) and type DHT11 (`DHTTYPE`).
-   Data read and Error Handling: The `updateSensorData` function reads temperature and humidity. It checks for `isnan()` to detect sensor read failures.
-   Retry logic (sensor): If a temporary read error occurs, an internal `sensor_error_count` is incremented, and the state transitions to `STATE_WAIT_INTERVAL` to retry later. If the error count reaches 5 (`MAX_SENSOR_ERRORS`), the system transitions to the critical STATE_ERROR.

### WiFi and TCP communications

-   Driver Library:The standard `WiFiS3.h` library is used for WiFi connectivity, leveraging the built-in module on the Arduino Uno R4 Wifi.

-   Transaction: The Sensor Package acts as a TCP client and uses a Single-Transaction-Request-Response pattern. The connection is established, data is sent, a brief response is expected, and the connection is immediately closed.

-   Data is sent as an unencrypted space separated string.

-   Connection Retry Logic:

    -   WiFi: `stateConnectWifi` continuously retries connection until successful or until a fatal driver error is detected.
    -   Server: `stateConnectServer` attempts connection and, on failure, transitions back to `STATE_CONNECT_WIFI` to re-validate the network before attempting to connect to the Control Unit/Server again.

-   Response Timeout and Disconnection: `STATE_WAIT_RESPONSE` enforces a 3-second timeout (`REPLY_TIMEOUT_MS`) for the Control Unit/server response. Regardless of success (response received) or failure (timeout), the connection is immediately closed (`drivers->client.stop()`) after the send/reply cycle, preparing for the next interval.

### Raw Data Format

The sensor data is formatted into a null-terminated string (`raw_payload`) before transmission using `snprintf()`.

The defined format is a space-separated string of the following values:

```
ID Temperature Humidity
```

The raw payload string is transmited to the Control Unit/Server for further parsing before forwarding it to the Azure App Service.

#### Example Payload Format

```
4 10.78 50.50
```

### Configuration Management

-   Static configuratin: Key parameters like timings, sensor defintions, and buffer sizes are defined as preprocessor constants in `SensorPackage.h`
-   Credentials: Network credentials (SSID and password) and the server's IP and port are managed in a separate `secrets.h` (based on `secrets.example.h`) for configuration management.

## Installation and Usage

This section guides you through how to set up the hardware, configuring the necessary secrets/configs, and compiling/uploading the firmware to the board.

### Hardware setup

| Component           | Connection                                                        | Purpose                                          |
| ------------------- | ----------------------------------------------------------------- | ------------------------------------------------ |
| DHT11 Sensor Module | Data pin connected to Arduino Digital Pin 7 (`DHTPIN`)            | Reads temperature and humidity                   |
| Arduino Uno R4 WiFi | (For prototype/testin/demo)USB-C cable connected to host computer | Power, programming, and Serial monitor debugging |

### Configuration and secrets

Before compilation, you must configure your network and server details in the `secret.h` file.

1. Find the `secrets.example.h` file in the `Arduino/include`.

2. Copy this file and rename the copy to `secrets.h`.

> Note: Do not commit `secrets.h` to git.

3. Edit `secrets.h` to define your settings:

```
#define SECRET_SSID "Your_WiFi_Network_Name"
#define SECRET_PASSWORD "Your_WiFi_Password"

// Server IP Address (Control Unit/Broker)
#define SERVER_IP1 192
// ... complete the octets
#define SERVER_PORT 8080
```

## How to compile and run

1. If you are in the root of the Project Repository
```cd ./Arduino```
2. Compiles the source code into firmware:
```pio run -e uno_r4_wifi```
3. Upload the compiled firmware to the connected board:
```pio run -e uno_r4_wifi -t upload```
4. Run the Serial monitor in the terminal:
```pio run -e uno_r4_wifi -t monitor```
> Note: the commands can be combined into
```pio run -e uno_r4_wifi -t upload -t monitor```

## Future Enhancements

-   Dynamic configuration: Implement logic to store WiFi credentials and configuration settings in EEPROM or Flash memory. This would allow the device to connect without hardcoding secrets, potentially enabling configuration mode like a captive portal or WiFi partitioning.
-   Enhance connection logic
-   Non-blocking WiFi connect
-   Interrupt based monitoring or sensor data transmission: Send notifications if sensor readings are above thresholds
-   Advances sensors: Upgrade the sensor reading to support more accurate sensors like DS18b20.
-   Client connection enhancement: Make the Sensor Package able to connect to the Control Unit as soon as it finds the Control Unit.Right now, it can only connect during its timed interval, and only if the Control Unit is listening before the Sensor Package is powered on.
