#include <SensorPackage.h>
#include <Arduino.h>
#include <string.h>

#ifndef SECRETS
#include "secrets.example.h"
#else
#include "secrets.h"
#endif

SensorPackage package = {{0, 0.0f, 0.0f}, STATE_INIT,      0,  0, 0, 0, SERVER_PORT,
                         SECRET_SSID,     SECRET_PASSWORD, {0}};

/* STATE LOOKUP TABLE */
StateFunction stateTable[] = {sensorInit, stateConnectWifi, stateConnectServer, stateWaitInterval,
                              stateSend,  stateWaitReply,   stateError};

/* HELPER FUNCTIONS */
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

void handleReply(SensorPackage *package, SensorDrivers *drivers)
{
    Serial.print("Server reply: ");
    while (drivers->client.available())
    {
        char c = drivers->client.read();
        Serial.print(c);
    }
    Serial.println();

    if (drivers->client.connected())
    {
        drivers->client.stop();
        Serial.println("Connection closed after receiving reply");
    }
    package->state = STATE_WAIT_INTERVAL;
}

void sensorInit(SensorPackage *package, SensorDrivers *drivers)
{
    drivers->dht.begin();
    package->data.id = 4;
    updateSensorData(package, drivers);
    package->state = STATE_CONNECT_WIFI;
}

void updateSensorData(SensorPackage *package, SensorDrivers *drivers)
{
    float hum = drivers->dht.readHumidity();
    float temp = drivers->dht.readTemperature();
    if (isnan(hum) || isnan(temp))
    {
        Serial.println("Error: Failed to read temperature and humidity DHT sensor.");
        package->state = STATE_WAIT_INTERVAL;
        return;
    }
    package->data.temperature = temp;
    package->data.humidity = hum;
}

void captureSensorData(SensorPackage *package, SensorDrivers *drivers)
{
    updateSensorData(package, drivers);

    if (package->state == STATE_WAIT_INTERVAL || package->state == STATE_ERROR)
    {
        Serial.println("Sensor error, skip data formatting until sensor issue is fixed.");
        return;
    }

    snprintf(package->rawPayload, sizeof(package->rawPayload), "%d %.2f %.2f", package->data.id,
             package->data.temperature, package->data.humidity);
    Serial.print("Data sent: ");
    Serial.println(package->rawPayload);
    package->state = STATE_CONNECT_SERVER;
}

/* STATE FUNCTIONS */

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
        package->state = STATE_CONNECT_WIFI;
    }
}

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

void stateSend(SensorPackage *package, SensorDrivers *drivers)
{
    Serial.println("Reading and sending sensor data\n");
    captureSensorData(package, drivers);

    if (package->state != STATE_CONNECT_SERVER)
        return;
    if (!drivers->client.connected())
    {
        Serial.println("Connection lost. Trying to connect again.");
        package->state = STATE_CONNECT_SERVER;
        return;
    }

    drivers->client.print(package->rawPayload);
    Serial.println("Data sent. Waiting for reply from Control Unit.");
    package->reply_wait_start = millis();
    package->state = STATE_WAIT_REPLY;
}

void stateWaitReply(SensorPackage *package, SensorDrivers *drivers)
{
    package->current_time = millis();
    if (drivers->client.available())
    {
        handleReply(package, drivers);
    }
    else if ((package->current_time - package->reply_wait_start) > REPLY_TIMEOUT_MS)
    {
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

void stateError(SensorPackage *package, SensorDrivers *drivers)
{
    Serial.println("ERROR STATE");
    delay(5000);
}

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
