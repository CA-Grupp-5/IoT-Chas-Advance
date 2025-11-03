#include <SensorIO.h>

WifiData   wifi_data(LOCAL_PORT, LOCAL_IP, SUBNET, GATEWAY, PRIMARY_DNS);
SensorData sensor_data;
size_t     bytes_read;

void setup()
{
    int i;

    Serial.begin(115200);

    for (i = 0; i < 7; i++) printStringDelay(1000, ".");

    wifiInit(&wifi_data);
}

void loop()
{
    runBroker(&wifi_data, &sensor_data);
}