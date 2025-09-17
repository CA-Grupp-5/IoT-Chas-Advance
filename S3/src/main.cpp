#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

WiFiClient client;
WiFiServer server(5000);
IPAddress local_IP(192, 168, 1, 184); // Ändra efter behov
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
// uint8_t buffer[4096];
uint8_t buffer[256];    // temporarily test a smaller buffer
char message[64];
size_t bytes_read;

void setup()
{
    Serial.begin(115200);
    int i = 0;
    while (i < 7)
    {
        Serial.print(".");
        delay(1000);
        ++i;
    }

    if (!WiFi.config(local_IP, gateway, subnet)) {
        Serial.println("Failed to config");
    }

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

        while (client.connected() && !client.available()) {
            delay(10);
        }

        if (client.available()) {
            bytes_read = client.read(buffer, sizeof(buffer) - 1);
            buffer[bytes_read] = '\0';

            snprintf(message, sizeof(message), "Server received %d bytes.", bytes_read);

            client.print(message);
            client.stop();

            Serial.println("Server is listening...");
        }
    }
    else
        delay(1000);
}
