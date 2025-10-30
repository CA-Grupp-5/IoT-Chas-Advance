#include <Arduino.h>
#include <SensorPackage.h>

void setup()
{
    sensorInit(&currentPackage);
    Serial.begin(115200);
    delay(2000);
    connectWifi();
    connectServer();
    last_sent = millis();
}

void loop()
{
    runClient();
}
