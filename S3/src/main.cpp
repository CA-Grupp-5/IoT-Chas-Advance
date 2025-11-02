#include <SensorIO.h>

WifiData wifi_data(5000, "192.168.1.184", "192.168.1.1", "255.255.255.0", "8.8.8.8");
SensorData sensor_data;
size_t     bytes_read;




void setup()
{
    int i;

    Serial.begin(115200);

    for (i = 0; i < 7; i++)
        printStringDelay(1000, ".");

    wifiInit(&wifi_data);
}




void loop()
{
    runBroker(&wifi_data, &sensor_data);
}