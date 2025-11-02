/**
 * @file main.cpp
 * @brief Main application for the Sensor Package Device
 * @version 0.1
 * @date 2025-11-02
 *
 * Defines all driver instances and uses the Sensor Package library to run a state driven
 * application to read temperature and humidity, establish WiFi connection, establish connection to
 * Control unit/server, and sending data.
 */
#include <Arduino.h>
#include <WiFi.h>
#include <SensorPackage.h>

/*****************************************
 * Load secrets for network credentials
 *****************************************/
#ifndef SECRETS
#include "secrets.example.h"
#else
#include "secrets.h"
#endif

/*****************************************
 * Global Drivers Instantiation
 *****************************************/
DHT        dht(DHTPIN, DHTTYPE);
WiFiClient client;
IPAddress  server(SERVER_IP1, SERVER_IP2, SERVER_IP3, SERVER_IP4);

/** Externally defined device state struct, initialized in SensorPackage.cpp */
extern SensorPackage package;

/**
 * @brief Drivers struct, initializing the references to the instances above
 */
SensorDrivers currentDrivers = {.dht = dht, .client = client, .esp32_broker = server};

/*****************************************
 * Main application
 *****************************************/

/**
 * @brief Runs once at startup
 *
 * Application is initialized to STATE_INIT in SensorPackage.cpp
 * Execution starts immediately in loop()
 *
 */
void setup()
{
    Serial.begin(115200);
}

/**
 * @brief The main execution loop. Calls the state machine function on every cycle.
 *
 */
void loop()
{
    runSensorPackage(&package, &currentDrivers);
}
