#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#define SECRETS

#ifndef SECRETS
#include "secrets.example.h"
#else
#include "secrets.h"
#endif

#include "backend_ca_cert.h"
#include <SensorIO.h>

SensorData sensor_data;
WiFiClient client;
WiFiServer server(5000);
IPAddress  local_IP(192, 168, 1, 184); // Ändra efter behov
IPAddress  gateway(192, 168, 1, 1);
IPAddress  subnet(255, 255, 255, 0);
IPAddress  primaryDNS(8, 8, 8, 8);

char   message[64];
size_t bytes_read;

const char *host = AZURE_HOST;
const int   https_port = AZURE_PORT;
const char *azure_root_ca = backend_root_ca;

void sendSensorLogs(const char *http_method);
void printResponseStatus(WiFiClientSecure &client, const char *http_method);

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

    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS))
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
        Serial.println("Sensor package connected.");

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
                // httpRequestFormat(&sensor_data, SIZE_BUF_SEND, host, https_port);

                snprintf(message, sizeof(message), "Server received %d bytes.\n", bytes_read);

                // just to check message from client
                Serial.println(sensor_data.buffer_send);
                /* Serial.print("Message from client: ");
                Serial.print(sensor_data.buffer_recv); */

                client.print(message);
                sendSensorLogs("POST");
                break;
            }
        }
        client.stop();
        Serial.println("Client disconnected. Server is listening...");
    }
    delay(100);
}

void sendSensorLogs(const char *http_method)
{
    WiFiClientSecure client;
    client.setCACert(azure_root_ca);

    Serial.printf("\nConnecting for %s to %s:%d\n", http_method, host, https_port);

    if (!client.connect(host, https_port))
    {
        Serial.println("Connection failed");
        return;
    }
    Serial.printf("Connection established to %s. Making %s request", host, http_method);

    int request = httpRequestFormat(&sensor_data, SIZE_BUF_SEND, host, https_port, http_method);

    Serial.printf("Sending %d bytes request\n", request);
    client.print(sensor_data.buffer_send);

    printResponseStatus(client, http_method);
    client.stop();
    Serial.println("Connection closed");
}

void printResponseStatus(WiFiClientSecure &client, const char *http_method)
{
    int success_status_code = 0;

    if (strcmp(http_method, "POST") == 0)
    {
        success_status_code = 201;
    }
    else
    {
        success_status_code = 200;
    }

    int attempts = 0;
    while (!client.available() && client.connected() && attempts < 100)
    {
        delay(10);
        ++attempts;
    }

    while (client.connected() || client.available())
    {
        while (client.available())
        {
            String response = client.readStringUntil('\n');
            if (response.startsWith("HTTP/1.1"))
            {
                Serial.printf("Response: %s\n", response.c_str());
                int received_status_code = response.substring(9).toInt();
                if (received_status_code == success_status_code)
                {
                    Serial.printf("Success: Sensor logs successfully %s (%d)\n", http_method,
                                  success_status_code);
                }
                else if (received_status_code >= 200 && received_status_code < 300)
                {
                    Serial.printf("Success: Request successfully sent with status code (%d)\n",
                                  received_status_code);
                }
                else if (received_status_code >= 300 && received_status_code < 400)
                {
                    Serial.printf("Success: Redirection with status code (%d)\n",
                                  received_status_code);
                }
                else if (received_status_code >= 400 && received_status_code < 500)
                {
                    Serial.printf("Client Error: %s with status code (%d)\n",
                                  ((strcmp(http_method, "PUT") == 0)
                                       ? "Invalid package ID or invalid input data"
                                       : "Bad request. Missing or invalid parameters"),
                                  received_status_code);
                }
                else if (received_status_code >= 500 && received_status_code <= 511)
                {
                    Serial.printf("Server error (%d)\n", received_status_code);
                }
                else
                {
                    Serial.printf("Error in general with status code (%d)\n", received_status_code);
                }
            }
            if (response.length() == 0)
                break;
        }
        if (!client.connected() || !client.available())
            break;
        delay(10);
    }
}
