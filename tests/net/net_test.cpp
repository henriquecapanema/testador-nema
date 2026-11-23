// Roda o firmware de verdade (motor + web + WebSocket) no PC, com relógio real e sockets TCP.
// Uso: net_test <porta> <segundos>   — imprime os vetores de SHA-1 e "PRONTO" quando está ouvindo.
#include "motion.h"
#include "log.h"
#include "samples.h"
#include "ws_server.h"
#include "web.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "esp_timer.h"
#include <chrono>
#include <deque>
#include <vector>
SerialT Serial; EspT ESP; WiFiT WiFi; MDNST MDNS; OTAT ArduinoOTA;
static auto T0=std::chrono::steady_clock::now();
static uint64_t nowUs(){return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-T0).count();}
uint32_t millis(){return nowUs()/1000;} uint32_t micros(){return (uint32_t)nowUs();}
size_t strlcpy(char*d,const char*s,size_t n){size_t l=strlen(s);if(n){size_t c=l<n-1?l:n-1;memcpy(d,s,c);d[c]=0;}return l;}
struct Q{int sz;std::deque<std::vector<uint8_t>> q;};
QueueHandle_t xQueueCreate(int,int s){auto*q=new Q;q->sz=s;return q;}
BaseType_t xQueueSend(QueueHandle_t h,const void*p,int){auto*q=(Q*)h;q->q.emplace_back((const uint8_t*)p,(const uint8_t*)p+q->sz);return 1;}
BaseType_t xQueueReceive(QueueHandle_t h,void*p,int){auto*q=(Q*)h;if(q->q.empty())return 0;memcpy(p,q->q.front().data(),q->sz);q->q.pop_front();return 1;}
int uxQueueMessagesWaiting(QueueHandle_t h){return ((Q*)h)->q.size();}
void vTaskDelay(int){} void xTaskCreatePinnedToCore(void(*)(void*),const char*,int,void*,int,void*,int){}
int esp_timer_create(const esp_timer_create_args_t*,esp_timer_handle_t*){return 0;} int esp_timer_start_periodic(esp_timer_handle_t,uint64_t){return 0;} int64_t esp_timer_get_time(){return nowUs();}
void sampleCb(void*); void wsPush(); void onWs(uint8_t,WsEvent,const char*,size_t);   // de samples.cpp e web.cpp (o webTask não roda aqui: chamamos à mão)

int main(int argc,char**argv){
  int port=argc>1?atoi(argv[1]):8181; double secs=argc>2?atof(argv[2]):20;
  char acc[29]; wsAcceptKey("dGhlIHNhbXBsZSBub25jZQ==",acc); printf("ACCEPT %s\n",acc);
  uint8_t d[20]; std::string big(1000,'a'); wsSha1((const uint8_t*)big.data(),big.size(),d);
  printf("SHA1000 "); for(auto x:d)printf("%02x",x); printf("\n");
  for(size_t n : {0,55,56,63,64,65}){std::string s(n,'x');wsSha1((const uint8_t*)s.data(),n,d);printf("SHA%zu ",n);for(auto x:d)printf("%02x",x);printf("\n");}
  loadMotorCfg();
  cmdQueue=xQueueCreate(32,sizeof(Cmd));
  wsBegin(port,onWs);                // no ESP32 é o webBegin() que faz isso, na porta 81
  printf("PRONTO\n"); fflush(stdout);
  uint64_t end=nowUs()+(uint64_t)(secs*1e6),nextS=0;
  while(nowUs()<end){
    motionService(); loopBeat=millis();
    wsLoop(); wsPush();
    if(nowUs()>=nextS){sampleCb(nullptr);nextS=nowUs()+10000;}
  }
  return 0;
}
