/**
 * @file States.h
 * @brief Defines the states of the Sensor Package state machine. The state machine controls the
 * operation flow, from initialization to sending data and handling errors.
 * @version 0.1
 * @date 2025-11-02
 *
 */
#ifndef STATES_H
#define STATES_H

typedef enum
{
    STATE_INIT, /**< Initial state, called once in the begining of program. Sets up the sensors */
    STATE_CONNECT_WIFI,   /**< Attempts to connect to the configured WiFi network. Retries if
                             connection fails */
    STATE_CONNECT_SERVER, /**< Attempts to establish a TCP connection to the ESP32 control unit
                             (server) */
    STATE_WAIT_INTERVAL, /**< Waits for the defined TRANSMISSION_INTERVAL_MS before sending the next
                            data payload */
    STATE_SEND, /**< Reads sensor data, formats the payload, and sends it to the connected control
                   unit/server */
    STATE_WAIT_RESPONSE, /**< Waits for a response from the control unit/server after sending data.
                            Timeout if the response is delayed longer than REPLY_TIMEOUT_MS */
    STATE_ERROR          /**< Error state after persistent sensor och connection failures */
} State;

#endif
