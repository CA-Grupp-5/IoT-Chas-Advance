#include <Arduino.h>
#include "WiFiS3.h"
#include "../lib/secrets.h"

WiFiClient client;
IPAddress server(SERVER_IP1, SERVER_IP2, SERVER_IP3, SERVER_IP4);
char ssid[ ] = SECRET_SSID;
char pass[ ] = SECRET_PASSWORD;
uint32_t start_time = 0;
const uint16_t port = SERVER_PORT;

void printWifiStatus();

void setup() {
  Serial.begin(115200);
  delay(500);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed");
    while (true);
  }
  Serial.print("Connecting client to WiFi");
  WiFi.begin(ssid, pass);

  start_time = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    if ((millis() - start_time) > 30000) {
      Serial.println("\nFailed to connect after 30s");
      while (true);
    }
  }

  printWifiStatus();

  Serial.print("Connected.\nClient IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("Connecting to server...");
  if (client.connect(server, port)) {
    Serial.println("Connected to server");
    client.println("GET / HTTP/1.1");
    client.println("Host: 192.168.1.189");
    client.println("Connection: close");
    client.println();
  }
  else {
    Serial.println("Connection to server failed");
  }
}

void loop() {
  if (client.connected() || client.available()) {
    while (client.available())
    {
      char c = client.read();
      Serial.print(c);
    }
  }
  else if (!client.connected() && !client.available()) {
    client.stop();
  }
  delay(1000);
}

void printWifiStatus() {
  Serial.println("\n---WIFI Status---");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());


  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.println("------");
}
