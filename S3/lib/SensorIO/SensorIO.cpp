#include "SensorIO.h"

int httpRequestFormat(SensorData *data, size_t size_buffer_send, const char *host, int port)
{
    return snprintf(data->buffer_send, size_buffer_send,
                    // "POST /packages/%d/logs HTTP/1.1\r\n"
                    "POST /packages/%d/logs HTTP/1.1\r\n"
                    "Host: %s\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: %d\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "%s",
                    data->id, host, data->length, data->http_body);
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
