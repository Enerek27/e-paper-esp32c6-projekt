#ifndef _DNS_SERVER_H_
#define _DNS_SERVER_H_

#include <esp_netif_ip_addr.h>

#ifdef __cplusplus
#include <string>
class DnsServer {
public:
DnsServer();
~DnsServer();
void Start(esp_ip4_addr_t gateway);
void Stop();
private:
int port_ = 53;
int fd_ = -1;
esp_ip4_addr_t gateway_;
void Run();
};
#endif

#ifdef __cplusplus
extern "C" {
    #endif

    void dns_server_start(uint32_t gateway_ip);

    #ifdef __cplusplus
}
#endif

#endif 