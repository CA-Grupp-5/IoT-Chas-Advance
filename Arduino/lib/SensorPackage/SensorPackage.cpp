#include <SensorPackage.h>

#ifndef SECRETS
#include "secrets.example.h"
#else
#include "secrets.h"
#endif

DHT            dht(DHTPIN, DHTTYPE);
WiFiClient     client;
IPAddress      server(SERVER_IP1, SERVER_IP2, SERVER_IP3, SERVER_IP4);
SensorPackage  currentPackage;
State          state = STATE_INIT;
uint32_t       current_time = 0;
uint32_t       last_sent = 0;
uint32_t       reply_wait_start = 0;
const uint16_t port = SERVER_PORT;
char           ssid[] = SECRET_SSID;
char           pass[] = SECRET_PASSWORD;
char           rawPayload[PAYLOAD_BUFFER_SIZE];

void connectWifi()
{
    uint32_t start_time;
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed");
        state = STATE_ERROR;
        return;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        printWifiStatus();
        Serial.print("Connected to wifi.\n");
        state = STATE_CONNECT_SERVER;
        return;
    }

    Serial.print("Connecting client to WiFi");
    WiFi.begin(ssid, pass);

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
    state = STATE_CONNECT_SERVER;
    printWifiStatus();
    Serial.println("Connected to WiFi");
}

void connectServer()
{
    if (client.connected())
    {
        Serial.println("Already connected to control unit.");
        state = STATE_SEND;
        return;
    }
    Serial.println("Connecting to Control Unit...");
    if (client.connect(server, port))
    {
        Serial.println("Successful connection to server.");
        state = STATE_SEND;
    }
    else
    {
        Serial.println("Connection to server failed. Will retry connection.");
        state = STATE_CONNECT_WIFI;
    }
}

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

void handleReply()
{
    Serial.print("Server reply: ");
    while (client.available())
    {
        char c = client.read();
        Serial.print(c);
    }

    if (client.connected())
    {
        client.stop();
        Serial.println("Connection closed after receiving reply");
    }
    Serial.println();
    state = STATE_WAIT_INTERVAL;
}

void sensorInit(SensorPackage *package)
{
    dht.begin();
    package->id = 4;
    updateSensorData(package);
    state = STATE_CONNECT_WIFI;
}

void updateSensorData(SensorPackage *package)
{
    float hum = dht.readHumidity();
    float temp = dht.readTemperature();
    if (isnan(hum) || isnan(temp))
    {
        Serial.println("Error: Failed to read temperature and humidity DHT sensor.");
        state = STATE_WAIT_INTERVAL;
        return;
    }
    package->temperature = temp;
    package->humidity = hum;
}

void captureSensorData()
{
    updateSensorData(&currentPackage);

    if (state == STATE_WAIT_INTERVAL || state == STATE_ERROR)
    {
        Serial.println("Sensor error, skip data formatting until sensor issue is fixed.");
        return;
    }

    snprintf(rawPayload, sizeof(rawPayload), "%d %.2f %.2f", currentPackage.id,
             currentPackage.temperature, currentPackage.humidity);
    Serial.print("Data sent: ");
    Serial.println(rawPayload);
    state = STATE_CONNECT_SERVER;
}

void runSensorPackage()
{
    switch (state)
    {
    case STATE_INIT:
        Serial.println("Initializing sensors and sensor package.");
        sensorInit(&currentPackage);
        break;

    case STATE_CONNECT_WIFI:
        Serial.println("Connecting to WiFi");
        connectWifi();
        break;

    case STATE_CONNECT_SERVER:
        connectServer();
        break;

    case STATE_WAIT_INTERVAL:
        current_time = millis();
        if (current_time - last_sent >= TRANSMISSION_INTERVAL_MS)
        {
            last_sent = current_time;
            state = STATE_SEND;
        }
        else
        {
            delay(10);
        }
        break;

    case STATE_SEND:
        Serial.println("Reading and sending sensor data");
        captureSensorData();

        if (state != STATE_SEND)
            break;

        if (!client.connected())
        {
            Serial.println("Connection lost. Trying to connect again.");
            state = STATE_CONNECT_SERVER;
            break;
        }

        client.print(rawPayload);
        Serial.println("Data sent. Waiting for reply from Control unit");
        reply_wait_start = millis();
        state = STATE_WAIT_REPLY;
        break;

    case STATE_WAIT_REPLY:
        current_time = millis();
        if (client.available())
        {
            handleReply();
        }
        else if ((current_time - reply_wait_start) > REPLY_TIMEOUT_MS)
        {
            Serial.println("Warning: Control Unit didn't respond in time.");
            if (client.connected())
            {
                client.stop();
            }
            state = STATE_WAIT_INTERVAL;
        }
        else
        {
            delay(10);
        }
        break;

    case STATE_ERROR:
        Serial.println("ERROR STATE: Persistent sensor failure.");
        delay(5000);
        break;

    default:
        break;
    }
}
