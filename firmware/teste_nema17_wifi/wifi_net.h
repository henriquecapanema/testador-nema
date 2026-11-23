// wifi_net.h — rede de casa salva na memória (NVS) e a rede própria "nemabot".
#pragma once
#include <Arduino.h>

extern String savedSsid;
extern bool staMode;
extern volatile uint32_t restartAt;

void loadWifi(String& ssid, String& pass);
void saveWifi(const String& ssid, const String& pass);
void forgetWifi();
void scheduleRestart();
void setupWifi();
