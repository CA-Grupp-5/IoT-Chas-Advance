#include <Arduino.h>
#include <WiFi.h>
#include <SensorPackage.h>

#ifndef SECRETS
#include "secrets.example.h"
#else
#include "secrets.h"
#endif

DHT        dht(DHTPIN, DHTTYPE);
WiFiClient client;
IPAddress  server(SERVER_IP1, SERVER_IP2, SERVER_IP3, SERVER_IP4);

extern SensorPackage package;

SensorDrivers currentDrivers = {.dht = dht, .client = client, .esp32_broker = server};

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    runSensorPackage(&package, &currentDrivers);
}
