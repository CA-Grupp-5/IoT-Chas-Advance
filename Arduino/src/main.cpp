#include <Arduino.h>
#include "WiFiS3.h"
#ifdef UNIT_TEST
#include "secrets.example.h"
#else
#include "secrets.h"
#endif

WiFiClient     client;
IPAddress      server(SERVER_IP1, SERVER_IP2, SERVER_IP3, SERVER_IP4);
char           ssid[] = SECRET_SSID;
char           pass[] = SECRET_PASSWORD;
uint32_t       start_time = 0;
uint32_t       current_time = 0;
uint32_t       time_left = 0;
uint32_t       last_sent = 0;
uint32_t       reply_wait_start = 0; /* timestamp when client starts to wait for server reply*/
const uint32_t reply_timeout = 3000;
const uint32_t interval = 60000;
const uint16_t port = SERVER_PORT;
bool           waiting_reply = false; // waiting for the server reply

const char *mock_data = "{\n"
                        "        \"package_id\": 12345678,\n"
                        "        \"temperature_c\": 30000.567,\n"
                        "        \"humidity_percent\": 987.123,\n"
                        "        \"battery\": 1024\n"
                        "    }\n";

void printWifiStatus();

void setup()
{
    Serial.begin(115200);
    delay(2000);

    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed");
        while (true);
    }
    Serial.print("Connecting client to WiFi");
    WiFi.begin(ssid, pass);

    start_time = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
        if ((millis() - start_time) > 30000)
        {
            Serial.println("\nFailed to connect after 30s");
            break;
        }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        printWifiStatus();
        Serial.print("Connected to wifi.\n");
    }
    else
    {
        Serial.println("WiFi connection failed");
    }

    Serial.println("Attempting first server connection...");
    if (client.connect(server, port))
    {
        Serial.println("Successful connection to server. Sending test message...");
        delay(200);
        Serial.print("First server reply: ");
        while (client.available())
        {
            char c = client.read();
            Serial.print(c);
        }
        Serial.println();
        client.stop();
        Serial.println("First connection closed. Ready for continuous communication");
    }
    else
    {
        Serial.println("Connection to server failed. Will retry connection.");
    }
    last_sent = millis();
    waiting_reply = false;
}

void loop()
{
    current_time = millis();

    static uint32_t last_countdown = 0;

    /* 5. after sending and closed connection, display countdown timer */
    if (current_time - last_countdown >= 1000)
    {
        last_countdown = current_time;
        if (current_time - last_sent < interval)
        {
            char countdown_message[40];
            time_left = (interval - (current_time - last_sent)) / 1000;
            snprintf(countdown_message, sizeof(countdown_message),
                     "\rNext data transfer in %u seconds ", time_left);
            Serial.print(countdown_message);
        }
    }

    /* 2. check server timeout */
    if (waiting_reply && (millis() - reply_wait_start) > reply_timeout)
    {
        Serial.println("Warning: Server didn't respond in time.");
        waiting_reply = false;
    }

    /* 3. when the client eventually receives the response from the server */
    if (waiting_reply && client.available())
    {
        Serial.print("Server reply: ");
        while (client.available())
        {
            char c = client.read();
            Serial.print(c);
        }
        Serial.println();
        waiting_reply = false;
    }

    /* 4. when the client is done waiting and the connection is still active ( = ready to close
     * connection) */
    if (!waiting_reply && client.connected())
    {
        client.stop();
        Serial.println("Connection closed by client");
    }

    /* 1. when it's time to send, and the client isn't still connected to the server, and it's not
     * waiting for a reply ( = still previous connection) */
    if ((current_time - last_sent >= interval) && !waiting_reply && !client.connected())
    {
        last_sent = current_time;

        Serial.println("\nTrying to establish connection...");
        if (client.connect(server, port))
        {
            Serial.println("Connection established. Sending data...");
            client.print(mock_data);
            Serial.println("Data sent. Waiting server reply");
            waiting_reply = true;
            reply_wait_start = millis();
        }
        else
        {
            Serial.print("Connection failed. Skipping data transfer.");
        }
    }
    delay(10);
}

void printWifiStatus()
{
    Serial.println("\n---WIFI Status---");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    IPAddress subnet = WiFi.subnetMask();
    Serial.print("Subnet: ");
    Serial.println(subnet);

    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
    Serial.println("------");
}
