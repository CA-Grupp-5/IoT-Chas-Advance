#ifndef MOCK_CLIENT_H
#define MOCK_CLIENT_H
#include "NetworkInterface.h"

class MockClient : public ClientInterface
{
  public:
    int connect(uint32_t ip, uint16_t port) override
    {
        return 3;
    }
    int available() override
    {
        return 1;
    }
    int read() override
    {
        return 'A';
    }
    void    stop() override {}
    uint8_t connected() override
    {
        return 1;
    }
};
#endif
