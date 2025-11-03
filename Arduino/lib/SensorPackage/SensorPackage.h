/**
 * @file SensorPackage.h
 * @brief Defintions for the Sensor Package library, including structs for state, drivers, and
 * lookup table for the state machine/states.
 * @version 0.1
 * @date 2025-11-02
 *
 */
#ifndef SENSOR_PACKAGE_H
#define SENSOR_PACKAGE_H

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFiS3.h>
#include "States.h"

/*****************************************
 * Configuration Constants
 *****************************************/
#define DHTPIN 7               /**< Configured pin for temp. and humidity sensor */
#define DHTTYPE DHT11          /**< Specifying type of DHTxx sensor module */
#define PAYLOAD_BUFFER_SIZE 64 /**< Max size for the raw sensor payload string */
#define MAX_SENSOR_ERRORS 5
#define SSID_SIZE 32 /**< Max size for the WiFi SSID character array */
#define PASS_SIZE 64 /**< Max size for the WiFi password character array */
#define TRANSMISSION_INTERVAL_MS \
    60000 /**< Time interval in milliseconds between successful data transmission */
#define REPLY_TIMEOUT_MS                                                                         \
    3000 /**< Timeout in milliseconds to wait for a response from the control unit/server before \
            retrying/disconnecting */

/*****************************************
 * Data Structures
 ******************************************/

/**
 * @brief Structure to hold the current sensor readings and device ID.
 */
typedef struct
{
    int   id;          /**< Unique ID of the sensor device */
    float temperature; /**< Latest temperature reading in Celsius */
    float humidity;    /**< Latest humidity reading in Celsius */
} PackageData;

/**
 * @brief Structure to hold all application state and configuration data.
 */
typedef struct
{
    PackageData    data;                /**< Current sensor readins */
    State          state;               /**< Current state of the application */
    uint32_t       current_time;        /**< Current value of millis() used for timing checks */
    uint32_t       last_sent;           /**< Timestamp of the last successful data transmission */
    uint32_t       response_wait_start; /**< Timestamp when device started to wait for response */
    const uint16_t port;                /**<  TCP/IP port number of the control unit/server*/
    uint8_t        sensor_error_count;  /**< Counter for sensor errors */
    char           ssid[SSID_SIZE];     /**< WiFi SSID string */
    char           pass[PASS_SIZE];     /**< WiFi password string */
    char raw_payload[PAYLOAD_BUFFER_SIZE]; /**< Buffer for the formatted sensor data string
                                              ("4 10.78 50.50") */
} SensorPackage;

/**
 * @brief Structure to hold all driver instances as references.
 * Used to pass external driver services to the library functions.
 */
typedef struct
{
    DHT        &dht;    /**< Reference to the external DHT sensor driver */
    WiFiClient &client; /**< Reference to the external WiFi client object for the TCP connection */
    IPAddress  &esp32_broker; /**<  Reference to the external IPAddress object for the control
                                 unit/server IP */
} SensorDrivers;

/*****************************************
 * Lookup Table for State machine
 *****************************************/

/**
 * @brief Typedef for the function signature used in the state machine lookup table. All state
 * functions must match this signature: void func(SensorPackage*, SensorDrivers*).
 */
typedef void (*StateFunction)(SensorPackage *package, SensorDrivers *drivers);

/*****************************************
 * Public API
 *****************************************/

/**
 * @brief Executes the current function in the state machine.
 * Repeatedly called in the Arduino loop() function to drive the application logic.
 *
 * @param package Pointer to the global device context struct (SensorPackage)
 * @param drivers Pointer to the global drivers struct (SensorDrivers)
 */
void runSensorPackage(SensorPackage *package, SensorDrivers *drivers);

/*****************************************
 * Internation State Function Prototypes
 *****************************************/

void printWifiStatus();
void handleResponse(SensorPackage *package, SensorDrivers *drivers);

void sensorInit(SensorPackage *package, SensorDrivers *drivers);
void updateSensorData(SensorPackage *package, SensorDrivers *drivers);
void captureSensorData(SensorPackage *package, SensorDrivers *drivers);

void stateConnectWifi(SensorPackage *package, SensorDrivers *drivers);
void stateConnectServer(SensorPackage *package, SensorDrivers *drivers);
void stateWaitInterval(SensorPackage *package, SensorDrivers *drivers);
void stateSend(SensorPackage *package, SensorDrivers *drivers);
void stateWaitResponse(SensorPackage *package, SensorDrivers *drivers);
void stateError(SensorPackage *package, SensorDrivers *drivers);

#endif
