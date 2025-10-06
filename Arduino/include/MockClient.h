#ifndef MOCK_CLIENT_H
#define MOCK_CLIENT_H
#include "NetworkInterface.h"

class MockClient : public ClientInterface
{
  public:
    int connect(uint32_t ip, uint16_t port) override
    {
        return 3; // atm, a bit lazy and simulating a successful connection
    }
    int available() override
    {
        return 1;
    }
    int read() override
    {
        return 'A'; // atm a bit lazy and simulating a single byte
    }
    void    stop() override {}
    uint8_t connected() override
    {
        return 1;
    }
};
#endif
