#ifndef SENSORIO_H
#define SENSORIO_H

#define SECRETS

#ifndef SECRETS
#include "secrets.example.h"
#else
#include "secrets.h"
#endif

#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "backend_ca_cert.h"

#define SIZE_BUF_SEND 256
#define SIZE_BODY 128
#define SIZE_BUF_RECV 64

/* Contains data from sensors and buffers for HTTP requests */
typedef struct
{
    int   id, length, result;
    float temperature, humidity;
    char  buffer_send[SIZE_BUF_SEND];
    char  http_body[SIZE_BODY];
    char  buffer_recv[SIZE_BUF_RECV];

} SensorData;

/* Flushes HTTP request buffers by filling them with blankspaces */
void buffersFlush(SensorData *data, size_t size_buf_send, size_t size_http_body,
    size_t size_buf_recv);

/* Extracts sensor values from a buffer and saves them into individual variables. Returns number of
 * values extracted. */
int valuesExtract(SensorData *data);

/* Formats a HTTP request body into a buffer. Returns body size. */
int httpBodyFormat(SensorData *data, size_t size_http_body);

/* Formats a request from a body and saves it into a buffer. Returns request size. */
// int httpRequestFormat(SensorData *data, size_t size_buffer_send);
int httpRequestFormat(SensorData *data, size_t size_buffer_send, const char *host, int port,
    const char *method);

void sensorLogsSend(SensorData *sensor_data, const char *method_http);
void responseStatusPrint(WiFiClientSecure &client, const char *method_http);
void printStringDelay(int delay_ms, const char *string);
size_t sensorDataRead(WiFiClient *client, SensorData *sensor_data);

#endif
