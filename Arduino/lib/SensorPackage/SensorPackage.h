#ifndef SENSOR_PACKAGE_H
#define SENSOR_PACKAGE_H

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFiS3.h>
#include "States.h"

#define DHTPIN 7
#define DHTTYPE DHT11
#define PAYLOAD_BUFFER_SIZE 64
#define SSID_SIZE 32
#define PASS_SIZE 64
#define TRANSMISSION_INTERVAL_MS 60000
#define REPLY_TIMEOUT_MS 3000

typedef struct
{
    int   id;
    float temperature;
    float humidity;
} PackageData;

typedef struct
{
    PackageData    data;
    State          state;
    uint32_t       current_time;
    uint32_t       time_left;
    uint32_t       last_sent;
    uint32_t       reply_wait_start;
    const uint16_t port;
    char           ssid[SSID_SIZE];
    char           pass[PASS_SIZE];
    char           rawPayload[PAYLOAD_BUFFER_SIZE];
} SensorPackage;

typedef struct
{
    DHT        &dht;
    WiFiClient &client;
    IPAddress  &esp32_broker;
} SensorDrivers;

/* STATE LOOKUP TABLE */
typedef void (*StateFunction)(SensorPackage *package, SensorDrivers *drivers);

/* HELPER FUNCTIONS */
void printWifiStatus();
void handleReply(SensorPackage *package, SensorDrivers *drivers);

/* SENSOR FUNCTIONS */
void sensorInit(SensorPackage *package, SensorDrivers *drivers);
void updateSensorData(SensorPackage *package, SensorDrivers *drivers);
void captureSensorData(SensorPackage *package, SensorDrivers *drivers);

/* STATE FUNCTIONS */
void stateConnectWifi(SensorPackage *package, SensorDrivers *drivers);
void stateConnectServer(SensorPackage *package, SensorDrivers *drivers);
void stateWaitInterval(SensorPackage *package, SensorDrivers *drivers);
void stateSend(SensorPackage *package, SensorDrivers *drivers);
void stateWaitReply(SensorPackage *package, SensorDrivers *drivers);
void stateError(SensorPackage *package, SensorDrivers *drivers);

void runSensorPackage(SensorPackage *package, SensorDrivers *drivers);

#endif
