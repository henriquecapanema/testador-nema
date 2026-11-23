#pragma once
#include "Arduino.h"
struct IPAddress{String toString()const{return "";}};
enum{WL_CONNECTED=3}; enum{WIFI_STA=1,WIFI_AP=2,WIFI_AP_STA=3};
#define WIFI_SCAN_RUNNING (-1)
#define WIFI_SCAN_FAILED (-2)
typedef enum{WIFI_AUTH_OPEN=0,WIFI_AUTH_WPA2_PSK=3} wifi_auth_mode_t;
struct WiFiT{String SSID(){return "";} String SSID(uint8_t){return "";} int8_t RSSI(){return 0;} int32_t RSSI(uint8_t){return 0;}
 IPAddress localIP(){return {};} IPAddress softAPIP(){return {};} int status(){return 0;} bool mode(int){return true;} void begin(const char*,const char*){}
 void setSleep(bool){} void setHostname(const char*){} int16_t scanComplete(){return 0;} int16_t scanNetworks(bool){return 0;} void scanDelete(){}
 wifi_auth_mode_t encryptionType(uint8_t){return WIFI_AUTH_OPEN;} int32_t channel(uint8_t){return 0;} bool disconnect(bool=false){return true;} bool softAP(const char*,const char*){return true;}};
extern WiFiT WiFi;
