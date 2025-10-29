#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFiS3.h>

#ifndef SECRETS
#include "secrets.example.h"
#else
#include "secrets.h"
#endif

#define DHTPIN 7
#define DHTTYPE DHT11
#define PAYLOAD_BUFFER_SIZE 64
#define TRANSMISSION_INTERVAL_MS 60000
#define REPLY_TIMEOUT_MS 3000
typedef struct
{
    int   id;
    float temperature;
    float humidity;
} SensorPackage;

DHT            dht(DHTPIN, DHTTYPE);
WiFiClient     client;
IPAddress      server(SERVER_IP1, SERVER_IP2, SERVER_IP3, SERVER_IP4);
SensorPackage  currentPackage;
uint32_t       current_time = 0;
uint32_t       time_left = 0;
uint32_t       last_sent = 0;
uint32_t       reply_wait_start = 0;
const uint16_t port = SERVER_PORT;
char           ssid[] = SECRET_SSID;
char           pass[] = SECRET_PASSWORD;
char           rawPayload[PAYLOAD_BUFFER_SIZE];
bool           waiting_reply = false;

void sensorInit(SensorPackage *package);
void connectWifi();
void connectServer();
void printWifiStatus();
void updateSensorData(SensorPackage *package, float *hum, float *temp);
void captureSensorData();
void runClient();

void setup()
{
    Serial.begin(115200);
    delay(2000);
    sensorInit(&currentPackage);
    connectWifi();
    connectServer();
    last_sent = millis();
}

void loop()
{
    runClient();
}

void connectWifi()
{
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed");
        while (true);
    }
    Serial.print("Connecting client to WiFi");
    WiFi.begin(ssid, pass);

    uint32_t start_time = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
        if ((millis() - start_time) > 30000)
        {
            Serial.println("\nFailed to connect after 30s");
            break;
        }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        printWifiStatus();
        Serial.print("Connected to wifi.\n");
    }
    else
    {
        Serial.println("WiFi connection failed");
    }
}

void connectServer()
{
    Serial.println("Connecting to Control Unit...");
    if (client.connect(server, port))
    {
        Serial.println("Successful connection to server. Sending test message...");
        delay(200);
        client.print("Sending test message to ESP32");
    }
    else
    {
        Serial.println("Connection to server failed. Will retry connection.");
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

void sensorInit(SensorPackage *package)
{
    dht.begin();
    package->id = 123456789;
    package->temperature = 0.0;
    package->humidity = 0.0;
}

void updateSensorData(SensorPackage *package, float *hum, float *temp)
{
    package->temperature = *(temp);
    package->humidity = *(hum);
}

void captureSensorData()
{
    float hum = dht.readHumidity();
    float temp = dht.readTemperature();
    if (isnan(hum) || isnan(temp))
    {
        Serial.println("Error: Failed to read from DHT sensor.");
        return;
    }
    updateSensorData(&currentPackage, &hum, &temp);
    snprintf(rawPayload, sizeof(rawPayload), "%d %.2f %.2f", currentPackage.id,
             currentPackage.temperature, currentPackage.humidity);
    Serial.print("Data sent: ");
    Serial.println(rawPayload);
}

void runClient()
{
    current_time = millis();

    /* 2. check server timeout */
    if (waiting_reply && (millis() - reply_wait_start) > REPLY_TIMEOUT_MS)
    {
        Serial.println("Warning: Server didn't respond in time.");
        waiting_reply = false;
    }

    /* 3. when the client eventually receives the response from the server */
    if (waiting_reply && client.available())
    {
        Serial.print("Server reply: ");
        while (client.available())
        {
            char c = client.read();
            Serial.print(c);
        }
        Serial.println();
        waiting_reply = false;
    }

    /* 4. when the client is done waiting and the connection is still active ( = ready to close
     * connection) */
    if (!waiting_reply && client.connected())
    {
        client.stop();
        Serial.println("Connection closed by client");
    }

    /* 1. when it's time to send, and the client isn't still connected to the server, and it's not
     * waiting for a reply ( = still previous connection) */
    if ((current_time - last_sent >= TRANSMISSION_INTERVAL_MS) && !waiting_reply &&
        !client.connected())
    {
        last_sent = current_time;

        captureSensorData();

        if (!client.connected())
        {
            Serial.println("\nTrying to establish connection...");
            if (!client.connect(server, SERVER_PORT))
            {
                Serial.print("Connection failed. Skipping data transfer.");
                return;
            }
            Serial.println("Reconnected!");
        }

        if (client.connected())
        {
            client.print(rawPayload);
            Serial.println("\nSending raw data");
            waiting_reply = true;
            reply_wait_start = current_time;
        }
        else
        {
            Serial.print("Connection failed. Skipping data transfer.");
        }
    }
    delay(10);
}