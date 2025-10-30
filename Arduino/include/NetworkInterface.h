#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H
#include <cstddef>
#include <stdint.h>

class NetworkInterface
{
  public:
    virtual int begin(const char *ssid, const char *passphrase) = 0;
    virtual int status() const = 0;
    virtual ~NetworkInterface(){};
};

class ClientInterface
{
  public:
    virtual int     connect(uint32_t ip, uint16_t port) = 0;
    virtual int     available() = 0;
    virtual int     read() = 0;
    virtual void    stop() = 0;
    virtual uint8_t connected() = 0;
    virtual ~ClientInterface(){};
};

#endif
