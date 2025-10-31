#include <SensorIO.h>

#define SECRETS
#ifndef SECRETS
#include "secrets.example.h"
#else
#include "secrets.h"
#endif

SensorData sensor_data;
WiFiClient client;
WiFiServer server(5000);
IPAddress  local_IP(192, 168, 1, 184);
IPAddress  gateway(192, 168, 1, 1);
IPAddress  subnet(255, 255, 255, 0);
IPAddress  primaryDNS(8, 8, 8, 8);
size_t     bytes_read;
char       message[64];




void setup()
{
    int i;

    Serial.begin(115200);


    for (i = 0; i < 7; i++)
        printStringDelay(1000, ".");

    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS))
        Serial.println("Failed to config");

    WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
    Serial.print("Connecting server to Wifi");

    while (WiFi.status() != WL_CONNECTED)
        printStringDelay(500, ".");
    Serial.print("Connected.\nServer IP: ");
    Serial.println(WiFi.localIP());

    server.begin();
    Serial.println("Server is listening...");
}




void loop()
{
    client = server.available();
    if (!client)
        goto skip;
    Serial.println("Sensor package connected.");

    while (client.connected())
    {
        if (!client.available())
            continue;

        buffersFlush(&sensor_data, SIZE_BUF_SEND, SIZE_BODY, SIZE_BUF_RECV);

        bytes_read = sensorDataRead(&client, &sensor_data);
        valuesExtract(&sensor_data);
        sensor_data.length = httpBodyFormat(&sensor_data, SIZE_BODY);
        Serial.println(sensor_data.buffer_send);

        snprintf(message, sizeof(message), "Server received %d bytes.\n", bytes_read);
        client.print(message);

        sensorLogsSend(&sensor_data, "POST");

        break;
    }
    client.stop();
    Serial.println("Client disconnected. Server is listening...");

skip:
    delay(100);
}
