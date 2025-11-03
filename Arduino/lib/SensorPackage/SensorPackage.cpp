/**
 * @file SensorPackage.cpp
 * @brief Implementation of the Sensor Package library.
 * Contains all state functions and the lookup table that drives the application
 * @version 0.1
 * @date 2025-11-02
 *
 */
#include <SensorPackage.h>
#include <Arduino.h>
#include <string.h>

/*****************************************
 * Load secrets for network credentials
 *****************************************/
#ifndef SECRETS
#include "secrets.example.h"
#else
#include "secrets.h"
#endif

/*****************************************
 * Global Device State Definition
 *****************************************/

/**
 * @brief Global instance of the SensorPackage struct, holding all mutable application state and
 * configurations. Initialized in sequential order (standard aggregate initialization)
 */
SensorPackage package = {
    {0, 0.0f, 0.0f}, /**< data (SensorData struct) */
    STATE_INIT,      /**< state (State enum) Start at initial state */
    0,               /**< current_time uint32_t */
    0,               /**< last_sent uint32_t */
    0,               /**< response_wait_start uint32_t */
    0,               /**< sensor_error_count uint8_t  */
    SERVER_PORT,     /**< port uint16_t */
    SECRET_SSID,     /**< ssid char[] */
    SECRET_PASSWORD, /**< password char[] */
    {0}              /**< raw_payload char[], zero-initialized buffer */
};

/*****************************************
 * State Lookup table
 *****************************************/

/**
 * @brief Array of function pointers, where index matches the State enum value.
 *
 */
StateFunction stateTable[] = {sensorInit, stateConnectWifi,  stateConnectServer, stateWaitInterval,
                              stateSend,  stateWaitResponse, stateError};

/*****************************************
 * Helper functions
 *****************************************/

/**
 * @brief Prints the current WiFi connection details to the Serial monitor
 */
void printWifiStatus()
{
    Serial.println("\n---WIFI Status---");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    IPAddress subnet = WiFi.subnetMask();
    Serial.print("Subnet: ");
    Serial.println(subnet);

    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
    Serial.println("------");
}

/**
 * @brief Handles the incoming response from the Control unit/server and closes the connection.
 *
 * @param package Pointer to the device state structure
 * @param drivers Pointer to the drivers structure
 */
void handleResponse(SensorPackage *package, SensorDrivers *drivers)
{
    Serial.print("Server response: ");
    while (drivers->client.available())
    {
        char c = drivers->client.read();
        Serial.print(c);
    }
    Serial.println();

    if (drivers->client.connected())
    {
        drivers->client.stop();
        Serial.println("Connection closed after receiving response");
    }
    package->state = STATE_WAIT_INTERVAL;
}

/**
 * @brief Reads temperature and humidity data from DHT sensor, handling potential errors.
 * If sensor read error occurs, error counter is incremented. If counter reaches the mac threshold
 * MAX_SENSOR_ERRORS, the state transitions to STATE_ERROR
 *
 * @param package Pointer to the device state structure
 * @param drivers Pointer to the drivers structure
 */
void updateSensorData(SensorPackage *package, SensorDrivers *drivers)
{
    float hum = drivers->dht.readHumidity();
    float temp = drivers->dht.readTemperature();
    bool  read_error = false;

    if (isnan(hum) || isnan(temp))
    {
        Serial.println("Error: Failed to read temperature and humidity DHT sensor.");
        read_error = true;
    }

    if (read_error)
    {
        ++package->sensor_error_count;
        Serial.print("Number of persistent sensor errors: ");
        Serial.println(package->sensor_error_count);
        if (package->sensor_error_count >= MAX_SENSOR_ERRORS)
        {
            package->state = STATE_ERROR;
        }
        else
        {
            /** Let's application waits the interval and try to read sensor again */
            package->state = STATE_WAIT_INTERVAL;
        }
        return;
    }

    package->sensor_error_count = 0;
    package->data.temperature = temp;
    package->data.humidity = hum;
}

/**
 * @brief Reads sensor data, formats it into the raw_payload buffer, and prepares the connection to
 * the Control Unit/server.
 *
 * @note Uses updateSensorData() to read and set the new sensor readings. If that function detects
 * error, the state transitions to STATE_WAITS_INTERVAL or STATE_ERROR and this function returns
 * early.
 *
 * @param package Pointer to the device state structure
 * @param drivers Pointer to the drivers structure
 */
void captureSensorData(SensorPackage *package, SensorDrivers *drivers)
{
    updateSensorData(package, drivers);

    if (package->state == STATE_WAIT_INTERVAL || package->state == STATE_ERROR)
    {
        Serial.println("Sensor error, skip data formatting until sensor issue is fixed.");
        return;
    }

    /** Formatted payload string: ID Temp Humidity */
    snprintf(package->raw_payload, sizeof(package->raw_payload), "%d %.2f %.2f", package->data.id,
             package->data.temperature, package->data.humidity);
    Serial.print("Data sent: ");
    Serial.println(package->raw_payload);
    package->state = STATE_CONNECT_SERVER;
}

/*****************************************
 * State Functions
 *****************************************/

/**
 * @brief Initial state: Called once in the beginning to set up sensor.
 * @note Init DHT drivers, sets the device ID, and starts reading and updating temperature and
 * humidity readings
 *
 * @param package Pointer to the device state structure
 * @param drivers Pointer to the drivers structure
 */
void sensorInit(SensorPackage *package, SensorDrivers *drivers)
{
    drivers->dht.begin();
    package->data.id = 4;
    updateSensorData(package, drivers);
    package->state = STATE_CONNECT_WIFI;
}

/**
 * @brief Attempts to connect to the configured WiFi network.
 * Retries connection repeatedly if unsuccessful, with a timeout limit. Transitions to STATE_ERROR
 * if errors are caused by hardware/modules/drivers
 *
 * @param package Pointer to the device state structure
 * @param drivers Pointer to the drivers structure
 */
void stateConnectWifi(SensorPackage *package, SensorDrivers *drivers)
{
    uint32_t start_time;
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed");
        package->state = STATE_ERROR;
        return;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        printWifiStatus();
        Serial.print("Connected to wifi.\n");
        package->state = STATE_CONNECT_SERVER;
        return;
    }

    Serial.print("Connecting client to WiFi");
    WiFi.begin(package->ssid, package->pass);

    start_time = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
        if ((millis() - start_time) > 30000)
        {
            Serial.println("\nFailed to connect after 30s");
            return;
        }
    }
    package->state = STATE_CONNECT_SERVER;
    printWifiStatus();
    Serial.println("Connected to WiFi");
}

/**
 * @brief Attempts to establish a TCP connection to the Control unit(server).
 * Uses the IP address from the drivers and the port from the package struct.
 *
 * @param package Pointer to the device state structure
 * @param drivers Pointer to the drivers structure
 */
void stateConnectServer(SensorPackage *package, SensorDrivers *drivers)
{
    if (drivers->client.connected())
    {
        Serial.println("Already connected to control unit.");
        package->state = STATE_SEND;
        return;
    }
    Serial.println("Connecting to Control Unit...");
    if (drivers->client.connect(drivers->esp32_broker, package->port))
    {
        Serial.println("Successful connection to server.");
        package->state = STATE_SEND;
    }
    else
    {
        Serial.println("Connection to server failed. Will retry connection.");
        /** If connection fails, go back to WiFi connect to ensure network connection is stable */
        package->state = STATE_CONNECT_WIFI;
    }
}

/**
 * @brief Waits for the defined transmission interval to elapse before moving to STATE_SEND
 *
 * @param package Pointer to the device state structure
 * @param drivers Pointer to the drivers structure
 */
void stateWaitInterval(SensorPackage *package, SensorDrivers *drivers)
{
    package->current_time = millis();
    if (package->current_time - package->last_sent >= TRANSMISSION_INTERVAL_MS)
    {
        package->last_sent = package->current_time;
        package->state = STATE_SEND;
    }
    else
    {
        delay(10);
    }
}

/**
 * @brief Collects the formatted and prepared data, sends the payload to the server, and starts
 * waiting for a response
 *
 * @param package Pointer to the device state structure
 * @param drivers Pointer to the drivers structure
 */
void stateSend(SensorPackage *package, SensorDrivers *drivers)
{
    captureSensorData(package, drivers);

    if (package->state != STATE_CONNECT_SERVER)
        /** Returns early if error state occured in captureSensorData */
        return;

    if (!drivers->client.connected())
    {
        /** Connection lost to Control unit, try again and return early */
        Serial.println("Connection lost. Trying to connect again.");
        package->state = STATE_CONNECT_SERVER;
        return;
    }

    drivers->client.print(package->raw_payload);
    Serial.println("Data sent. Waiting for reply from Control Unit.");
    package->response_wait_start = millis();
    package->state = STATE_WAIT_RESPONSE;
}

/**
 * @brief Waits for available data (response) from the Control Unit/server or times out if response
 * is abscent or delayed
 *
 * @param package Pointer to the device state structure
 * @param drivers Pointer to the drivers structure
 */
void stateWaitResponse(SensorPackage *package, SensorDrivers *drivers)
{
    package->current_time = millis();
    if (drivers->client.available())
    {
        handleResponse(package, drivers);
    }
    else if ((package->current_time - package->response_wait_start) > REPLY_TIMEOUT_MS)
    {
        /** Timeout if response is abscent, and try again next interval */
        Serial.println("Warning: Control Unit didn't respond in time.");
        if (drivers->client.connected())
        {
            drivers->client.stop();
        }
        package->state = STATE_WAIT_INTERVAL;
    }
    else
    {
        delay(10);
    }
}

/**
 * @brief Critical error state, where the system halts and print an error message
 *
 * @param package Pointer to the device state structure
 * @param drivers Pointer to the drivers structure
 */
void stateError(SensorPackage *package, SensorDrivers *drivers)
{
    Serial.println("ERROR STATE");
    delay(5000);
}

/*****************************************
 * State machine
 *****************************************/

/**
 * @brief Executes the current state function based on package->state
 *
 * Checks for a valid state index before execution to prevent out-of-bounds access.
 *
 * @param package Pointer to the device state structure
 * @param drivers Pointer to the drivers structure
 */
void runSensorPackage(SensorPackage *package, SensorDrivers *drivers)
{
    if (package->state < sizeof(stateTable) / sizeof(stateTable[0]))
    {
        stateTable[package->state](package, drivers);
    }
    else
    {
        Serial.println("ERROR: Invalid state index.");
        package->state = STATE_ERROR;
    }
}
