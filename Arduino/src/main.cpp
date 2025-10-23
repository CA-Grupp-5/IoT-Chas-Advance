#include <Arduino.h>
#include "WiFiS3.h"

#define SECRETS

#ifndef SECRETS
#include "secrets.example.h"
#else
#include "secrets.h"
#endif

WiFiClient client;
IPAddress server(SERVER_IP1, SERVER_IP2, SERVER_IP3, SERVER_IP4);
char ssid[ ] = SECRET_SSID;
char pass[ ] = SECRET_PASSWORD;
uint32_t start_time = 0;
uint32_t current_time = 0;
uint32_t time_left = 0; // mostly for checking time between transfer
uint32_t last_sent = 0;
const uint32_t interval = 60000;
const uint16_t port = SERVER_PORT;

const char* mock_data = "4 123 756";

void printWifiStatus();

void setup()
{
    Serial.begin(115200);
    delay(2000);

    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed");
        while (true);
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
            while (true);
        }
    }

    printWifiStatus();

    Serial.print("Connected to wifi.\n");

    Serial.println("Connecting to server...");
    if (client.connect(server, port))
    {
        Serial.println("Connected to server");
        client.print("Sending a test message to the server!\n");
    }
    else
    {
        Serial.println("Connection to server failed");
    }
    last_sent = millis();
}

void loop()
{
    current_time = millis();

    // this is mostly to check time between data transfer, may remove later?
    static uint32_t last_countdown = 0;
    if (current_time - last_countdown >= 1000)
    {
        last_countdown = current_time;
        if (current_time - last_sent < interval)
        {
            time_left = (interval - (current_time - last_sent)) / 1000;
            Serial.print("\rNext data transfer in ");
            Serial.print(time_left);
            Serial.print(" seconds");
            if (time_left == 0)
            {
                Serial.println();
            }
        }
    }

    if ((current_time - last_sent >= interval))
    {
        last_sent = current_time;

        while (client.available())
        {
            char c = client.read();
            Serial.print(c);
        }
        if (!client.connected())
        {
            Serial.println("\nTrying to reconnect to ESP32");
            if (client.connect(server, port))
            {
                Serial.println("Reconnected to ESP32");
            }
            else
            {
                Serial.println("Reconnection failed");
                return;
            }
        }
        if (client.connected())
        {
            client.print(mock_data);
            Serial.println("\nSending mock data to ESP32\n");
            Serial.print(mock_data);
        }
        else
        {
            Serial.println("Cannot send data, not connected to server.");
        }
    }
    delay(10);
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
