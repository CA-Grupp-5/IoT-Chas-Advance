#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "backend_ca_cert.h"
#include <SensorIO.h>

/* #define SECRETS */

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
const int  https_port = AZURE_PORT;
char       message[64];
const char *host = AZURE_HOST;
const char *azure_root_ca = backend_root_ca;

void sensorLogsSend(const char *method_http);
void responseStatusPrint(WiFiClientSecure &client, const char *method_http);
void printStringDelay(int delay_ms, char *string);




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
    
        bytes_read = client.read
        (
            (uint8_t *) sensor_data.buffer_recv,
            (size_t) (sizeof(sensor_data.buffer_recv) - 1)
        );
        sensor_data.buffer_recv[bytes_read] = '\0';
    
        valuesExtract(&sensor_data);
        sensor_data.length = httpBodyFormat(&sensor_data, SIZE_BODY);
        Serial.println(sensor_data.buffer_send);
        
        snprintf(message, sizeof(message), "Server received %d bytes.\n", bytes_read);
        client.print(message);

        sensorLogsSend("POST");
    
        break;
    }
    client.stop();
    Serial.println("Client disconnected. Server is listening...");

skip:
    delay(100);
}




void printStringDelay(int delay_ms, char *string)
{
    Serial.print(string);
    delay(delay_ms);
}

void sensorLogsSend(const char *method_http)
{
    WiFiClientSecure client;
    int request;

    client.setCACert(azure_root_ca);
    Serial.printf("\nConnecting for %s to %s:%d\n", method_http, host, https_port);

    if (!client.connect(host, https_port))
    {
        Serial.println("Connection failed");
        return;
    }
    Serial.printf("Connection established to %s. Making %s request", host, method_http);

    request = httpRequestFormat(&sensor_data, SIZE_BUF_SEND, host, https_port, method_http);
    Serial.printf("Sending %d bytes request\n", request);
    client.print(sensor_data.buffer_send);

    responseStatusPrint(client, method_http);
    client.stop();
    Serial.println("Connection closed");
}

void responseStatusPrint(WiFiClientSecure &client, const char *method_http)
{
    int success_status_code = 0;
    int attempts = 0;

    if (strcmp(method_http, "POST") == 0)
        success_status_code = 201;
    else
        success_status_code = 200;

    while (!client.available() && client.connected() && attempts < 100)
    {
        delay(10);
        ++attempts;
    }

    while (client.connected() || client.available())
    {
        while (client.available())
        {
            String response;
            int received_status_code;

            response = client.readStringUntil('\n');
            if (!response.startsWith("HTTP/1.1"))
                goto skipahead;

            Serial.printf
            (
                "Response: %s\n", 
                response.c_str()
            );

            received_status_code = response.substring(9).toInt();
            if (received_status_code == success_status_code)
            {
                Serial.printf
                (
                    "Success: Sensor logs successfully %s (%d)\n",
                    method_http,
                    success_status_code
                );
            
            }
            else if (received_status_code >= 200 && received_status_code < 300)
            {
                Serial.printf
                (
                    "Success: Request successfully sent with status code (%d)\n",
                    received_status_code
                );
            
            }
            else if (received_status_code >= 300 && received_status_code < 400)
            {
                Serial.printf
                (
                    "Success: Redirection with status code (%d)\n",
                    received_status_code
                );
                
            }
            else if (received_status_code >= 400 && received_status_code < 500)
            {
                Serial.printf
                (
                    "Client Error: %s with status code (%d)\n",
                    (
                        (strcmp(method_http, "PUT") == 0)
                            ? "Invalid package ID or invalid input data"
                            : "Bad request. Missing or invalid parameters"
                    ),
                    received_status_code
                );
            
            }
            else if (received_status_code >= 500 && received_status_code <= 511)
                Serial.printf("Server error (%d)\n", received_status_code);
            else
                Serial.printf("Error in general with status code (%d)\n", received_status_code);
                
        skipahead:
            if (response.length() == 0)
                break;
        }

        if (!client.connected() || !client.available())
            break;
        delay(10);
    }

}