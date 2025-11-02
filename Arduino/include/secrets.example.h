/**
 * @file secrets.example.h
 * @brief Template for defining network and server configuration constants
 * For local development, copy this file to a new file named "secrets.h" and fill in with your
 * values
 *
 * @note DO NOT COMMIT THIS FILE IF IT CONTAINS SECRET CREDENTIALS
 * @version 0.1
 * @date 2025-11-02
 *
 */
#ifndef SECRETS_EXAMPLE_H
#define SECRETS_EXAMPLE_H

/** --- WiFi Network Credentials --- */
#define SECRET_SSID "your_wifi_ssid"
#define SECRET_PASSWORD "your_wifi_password"

/** --- Server (Control Unit) Configuration --- */

/** The IP address of the server (ESP32 broker) the sensor package will establish a connection to.
 * Written as four octats. Format: XXX XXX X XXX
 */
#define SERVER_IP1 000
#define SERVER_IP2 000
#define SERVER_IP3 0
#define SERVER_IP4 000

/** The TCP port number the server is listening on for incoming data */
#define SERVER_PORT 0000
#endif
