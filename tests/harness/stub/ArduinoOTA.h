#pragma once
#include "Arduino.h"
typedef enum{OTA_AUTH_ERROR,OTA_BEGIN_ERROR,OTA_CONNECT_ERROR,OTA_RECEIVE_ERROR,OTA_END_ERROR} ota_error_t;
struct OTAT{OTAT& setHostname(const char*){return *this;} OTAT& setPassword(const char*){return *this;}
 OTAT& onStart(std::function<void()>){return *this;} OTAT& onEnd(std::function<void()>){return *this;}
 OTAT& onProgress(std::function<void(unsigned int,unsigned int)>){return *this;} OTAT& onError(std::function<void(ota_error_t)>){return *this;}
 void begin(){} void handle(){}}; extern OTAT ArduinoOTA;
