#include <Arduino.h>
#include <WiFi.h>

#define SECRETS

#ifndef SECRETS
#include "secrets.example.h"
#else
#include "secrets.h"
#endif

#include <SensorIO.h>

SensorData sensor_data;
WiFiClient client;
WiFiServer server(5000);
IPAddress  local_IP(192, 168, 1, 184); // Ändra efter behov
IPAddress  gateway(192, 168, 1, 1);
IPAddress  subnet(255, 255, 255, 0);
char       message[64];
size_t     bytes_read;

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

    if (!WiFi.config(local_IP, gateway, subnet))
    {
        Serial.println("Failed to config");
    }

    // ssid and password should be placed in secrets.h
    // WiFi.begin(ssid, password);
    WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
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

        while (client.connected())
        {
            if (client.available())
            {
                buffersFlush(&sensor_data, SIZE_BUF_SEND, SIZE_BODY, SIZE_BUF_RECV);

                bytes_read = client.read((uint8_t *) sensor_data.buffer_recv,
                                         (size_t) (sizeof(sensor_data.buffer_recv) - 1));
                sensor_data.buffer_recv[bytes_read] = '\0';
                valuesExtract(&sensor_data);
                sensor_data.length = httpBodyFormat(&sensor_data, SIZE_BODY);
                httpRequestFormat(&sensor_data, SIZE_BUF_SEND);

                snprintf(message, sizeof(message), "Server received %d bytes.\n", bytes_read);

                // just to check message from client
                Serial.println(sensor_data.buffer_send);
                /* Serial.print("Message from client: ");
                Serial.print(sensor_data.buffer_recv); */

                // send back response to client
                client.print(message);
                client.stop();

                Serial.println("Server is listening...");
            }
        }
        client.stop();
        Serial.println("Client disconnected. Server is listening...");
    }

    /*time_since_last_message = millis() - last_client_message_time;
    if (time_since_last_message > (interval + time_padding) && last_client_message_time != 0)
    {
        Serial.print("Warning. Client message overdue. Time since last message: ");
        Serial.print(time_since_last_message);
        Serial.println(" ms.");
    }*/

    delay(100);
}
