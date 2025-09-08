#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

WiFiClient client;
WiFiServer server(5000);
uint8_t buffer[4096];
char message[64];
size_t bytes_read;

void setup()
{
    Serial.begin(115200);

    // ssid and password should be placed in secrets.h
    WiFi.begin(ssid, password);
    Serial.print("Connecting server to Wifi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.print("Connected.\nServer IP: ");
    Serial.println(WiFi.localIP());

    server.begin();

    Serial.println("Server is listening...");
}

void loop()
{
    client = server.available();
    if (client)
    {
        Serial.println("Client connected.");
        bytes_read = client.read(buffer, sizeof(buffer) - 1);
        buffer[bytes_read] = '\0';

        snprintf(message, sizeof(message), "Server received %d bytes.", bytes_read);

        client.print(message);
        client.stop();
        
        Serial.println("Server is listening...");
    }
    else
        delay(1000);
}