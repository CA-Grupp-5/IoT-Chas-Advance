#ifndef MOCK_NETWORK_H
#define MOCK_NETWORK_H
#include "NetworkInterface.h"
#include "WiFiMockType.h"

class MockNetwork : public NetworkInterface
{
  public:
    int current_status = MOCK_WL_IDLE_STATUS;
    int begin(const char *ssid, const char *passphrase) override
    {
        current_status = MOCK_WL_CONNECTED;
        return current_status;
    };
    int status() const override
    {
        return current_status;
    }
};
#endif
