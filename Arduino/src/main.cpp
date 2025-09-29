#include "WiFiS3.h"
#ifdef UNIT_TEST
#include "secrets.example.h"
#else
#include "secrets.h"
#endif
#include <Arduino.h>

WiFiClient client;
IPAddress  server(SERVER_IP1, SERVER_IP2, SERVER_IP3, SERVER_IP4);
char       ssid[] = SECRET_SSID;
char       pass[] = SECRET_PASSWORD;
uint32_t   start_time = 0;
// unint32_t last_sent = 0; // gonna try this instead of delay
const uint16_t port = SERVER_PORT;

void printWifiStatus();

void setup()
{
    Serial.begin(115200);
    delay(2000);

    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed");
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
        }
    }

    printWifiStatus();

    Serial.print("Connected.\n");

    Serial.println("Connecting to server...");
    if (client.connect(server, port))
    {
        Serial.println("Connected to server");
        client.print("Sending a test message to the server!\n");
        // just want to check why initial request is 39 bytes, and then 42 bytes
        Serial.print("Sending a test message to the server!\n");
        Serial.print(strlen("Sending a test message to the server!\n"));
    }
    else
    {
        Serial.println("Connection to server failed");
    }
}

void loop()
{
    if (client.connected())
    {
        while (client.available())
        {
            char c = client.read();
            Serial.print(c);
        }
    }
    else
    {
        Serial.println("\nReconnecting...\n");
        if (client.connect(server, port))
        {
            client.print("Sending a new test message to the server!\n");
            Serial.print(strlen("Sending a test message to the server!\n"));
            unsigned long start = millis();
            while (!client.available() && millis() - start < 2000)
            {
                delay(10);
            }

            while (client.available())
            {
                char c = client.read();
                Serial.print(c);
            }
            Serial.println();
        }
        else
        {
            Serial.println("Connection to server failed");
        }
        client.stop();
    }
    // next: request intervall 60000 = 60 secs. Loop is going to be blocked :/ How will it affect
    // server? Probably not ideal to slap a delay??
    delay(3000);
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
