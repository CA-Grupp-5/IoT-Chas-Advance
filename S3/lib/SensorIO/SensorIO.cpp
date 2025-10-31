#include "SensorIO.h"

int httpRequestFormat(SensorData *data, size_t size_buffer_send, const char *host, int port,
                      const char *method)
{
    return snprintf(data->buffer_send, size_buffer_send,
                    // "POST /packages/%d/logs HTTP/1.1\r\n"
                    "%s /packages/%d/logs HTTP/1.1\r\n"
                    "Host: %s\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: %d\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "%s",
                    method, data->id, host, data->length, data->http_body);
}

int httpBodyFormat(SensorData *data, size_t size_http_body)
{
    return snprintf(data->http_body, size_http_body,
                    "{\r\n"
                    "    \"temperature\": %.2f,\r\n"
                    "    \"humidity\": %.2f\r\n"
                    "}",
                    data->temperature, data->humidity);
}

int valuesExtract(SensorData *data)
{
    return sscanf(data->buffer_recv, "%d %f %f", &data->id, &data->temperature, &data->humidity);
}

void buffersFlush(SensorData *data, size_t size_buf_send, size_t size_http_body,
                  size_t size_buf_recv)
{
    memset(&data->buffer_send, ' ', size_buf_send);
    memset(&data->http_body, ' ', size_http_body);
    memset(&data->buffer_recv, ' ', size_buf_recv);
}




size_t sensorDataRead(WiFiClient *client, SensorData *sensor_data)
{
    size_t bytes_read = client->read
    (
        (uint8_t *) sensor_data->buffer_recv,
        (size_t) (sizeof(sensor_data->buffer_recv) - 1)
    );
    sensor_data->buffer_recv[bytes_read] = '\0';

    return bytes_read;
}

void printStringDelay(int delay_ms, const char *string)
{
    Serial.print(string);
    delay(delay_ms);
}

void sensorLogsSend(SensorData *sensor_data, const char *method_http)
{
    WiFiClientSecure client;
    int request;
    const int  https_port = AZURE_PORT;
    const char *host = AZURE_HOST;
    const char *azure_root_ca = BACKEND_CA_ROOT;

    client.setCACert(azure_root_ca);
    Serial.printf("\nConnecting for %s to %s:%d\n", method_http, host, https_port);

    if (!client.connect(host, https_port))
    {
        Serial.println("Connection failed");
        return;
    }
    Serial.printf("Connection established to %s. Making %s request", host, method_http);

    request = httpRequestFormat(sensor_data, SIZE_BUF_SEND, host, https_port, method_http);
    Serial.printf("Sending %d bytes request\n", request);
    client.print(sensor_data->buffer_send);

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
                goto skip_responseStatusPrint;

            Serial.printf("Response: %s\n", response.c_str());

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
                
        skip_responseStatusPrint:
            if (response.length() == 0)
                break;
        }

        if (!client.connected() || !client.available())
            break;
        delay(10);
    }

}