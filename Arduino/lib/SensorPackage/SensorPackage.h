#ifndef SENSOR_PACKAGE_H
#define SENSOR_PACKAGE_H

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFiS3.h>

#define DHTPIN 7
#define DHTTYPE DHT11
#define PAYLOAD_BUFFER_SIZE 64
#define TRANSMISSION_INTERVAL_MS 60000
#define REPLY_TIMEOUT_MS 3000

typedef struct
{
    int   id;
    float temperature;
    float humidity;
} SensorPackage;

extern DHT            dht;
extern WiFiClient     client;
extern IPAddress      server;
extern SensorPackage  currentPackage;
extern uint32_t       current_time;
extern uint32_t       time_left;
extern uint32_t       last_sent;
extern uint32_t       reply_wait_start;
extern const uint16_t port;
extern bool           waiting_reply;
extern char           ssid[];
extern char           pass[];
extern char           rawPayload[PAYLOAD_BUFFER_SIZE];
extern const char    *mock_data;

void connectWifi();
void connectServer();
void printWifiStatus();
void sensorInit(SensorPackage *package);
void updateSensorData(SensorPackage *package, float *hum, float *temp);
void captureSensorData();
void runClient();

#endif
