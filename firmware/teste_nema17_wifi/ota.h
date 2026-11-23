// ota.h — atualização do código pelo Wi-Fi (ArduinoOTA), com senha salva na NVS.
#pragma once
#include <Arduino.h>

extern bool otaOn;
String loadOtaPass();
void saveOtaPass(const String& pass);
void setupOta();
