// WiFi.h de teste: o mesmo stub do harness, mais WiFiServer/WiFiClient de verdade (sockets TCP),
// para rodar o servidor WebSocket do firmware no PC e conectar um navegador nele.
#pragma once
#include "Arduino.h"
#include <memory>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
struct IPAddress{String toString()const{return "127.0.0.1";}};
enum{WL_CONNECTED=3}; enum{WIFI_STA=1,WIFI_AP=2,WIFI_AP_STA=3};
#define WIFI_SCAN_RUNNING (-1)
#define WIFI_SCAN_FAILED (-2)
typedef enum{WIFI_AUTH_OPEN=0,WIFI_AUTH_WPA2_PSK=3} wifi_auth_mode_t;
struct WiFiT{String SSID(){return "teste";} String SSID(uint8_t){return "";} int8_t RSSI(){return -50;} int32_t RSSI(uint8_t){return 0;}
 IPAddress localIP(){return {};} IPAddress softAPIP(){return {};} int status(){return WL_CONNECTED;} bool mode(int){return true;} void begin(const char*,const char*){}
 void setSleep(bool){} void setHostname(const char*){} int16_t scanComplete(){return 0;} int16_t scanNetworks(bool){return 0;} void scanDelete(){}
 wifi_auth_mode_t encryptionType(uint8_t){return WIFI_AUTH_OPEN;} int32_t channel(uint8_t){return 0;} bool disconnect(bool=false){return true;} bool softAP(const char*,const char*){return true;}};
extern WiFiT WiFi;

class WiFiClient {
  std::shared_ptr<int> sp;
public:
  WiFiClient(){} explicit WiFiClient(int f):sp(std::make_shared<int>(f)){}
  uint8_t connected(){ return 0;   // como no ESP32 com errno sobrando: o firmware não pode depender disto
  }
  uint8_t connectedReal(){
    if(!sp||*sp<0)return 0;
    char c;ssize_t r=recv(*sp,&c,1,MSG_PEEK|MSG_DONTWAIT);
    return r>0||(r<0&&(errno==EAGAIN||errno==EWOULDBLOCK));
  }
  int available(){if(!sp||*sp<0)return 0;int n=0;ioctl(*sp,FIONREAD,&n);return n;}
  int read(uint8_t* b,size_t n){if(!sp||*sp<0)return -1;ssize_t r=recv(*sp,b,n,MSG_DONTWAIT);return r<0?-1:(int)r;}
  size_t write(const uint8_t* b,size_t n){
    if(!sp||*sp<0)return 0;
    size_t o=0;
    while(o<n){ssize_t r=send(*sp,b+o,n-o,MSG_NOSIGNAL);if(r<0){if(errno==EAGAIN||errno==EINTR){usleep(100);continue;}break;}o+=r;}
    return o;
  }
  int fd()const{return sp?*sp:-1;}
  void stop(){if(sp&&*sp>=0){close(*sp);*sp=-1;}}
  void setNoDelay(bool on){if(sp&&*sp>=0){int v=on;setsockopt(*sp,IPPROTO_TCP,TCP_NODELAY,&v,sizeof v);}}
};

class WiFiServer {
  int port,lfd=-1,pend=-1;
public:
  explicit WiFiServer(uint16_t p):port(p){}
  void begin(){
    lfd=socket(AF_INET,SOCK_STREAM,0);int one=1;setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&one,sizeof one);
    sockaddr_in a{};a.sin_family=AF_INET;a.sin_port=htons(port);a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    if(bind(lfd,(sockaddr*)&a,sizeof a)||listen(lfd,8)){perror("bind/listen");exit(2);}
    fcntl(lfd,F_SETFL,O_NONBLOCK);
  }
  void setNoDelay(bool){}
  bool hasClient(){if(pend<0&&lfd>=0)pend=accept(lfd,nullptr,nullptr);return pend>=0;}
  WiFiClient available(){if(!hasClient())return WiFiClient();int f=pend;pend=-1;return WiFiClient(f);}
};
