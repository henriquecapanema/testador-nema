// wifi_net.cpp — conexão Wi-Fi sem senha no código.
#include "wifi_net.h"
#include "config.h"
#include "log.h"
#include <WiFi.h>
#include <Preferences.h>

String savedSsid;                 // rede salva ("" = nenhuma)
bool staMode = false;             // true = conectado na rede de casa
volatile uint32_t restartAt = 0;  // reinício agendado (para dar tempo de responder a página)

void loadWifi(String& ssid, String& pass) {
  Preferences p;
  p.begin(PREFS_NS, true);
  ssid = p.getString("ssid", "");
  pass = p.getString("pass", "");
  p.end();
}

void saveWifi(const String& ssid, const String& pass) {
  Preferences p;
  p.begin(PREFS_NS, false);
  p.putString("ssid", ssid);
  p.putString("pass", pass);
  p.end();
  logPrintf("Wi-Fi salvo: \"%s\". Reiniciando…\n", ssid.c_str());
}

void forgetWifi() {
  Preferences p;
  p.begin(PREFS_NS, false);
  p.clear();
  p.end();
  logPrintln("Wi-Fi esquecido. Reiniciando…");
}

void scheduleRestart() { restartAt = millis() + 1500; if (restartAt == 0) restartAt = 1; }

void setupWifi() {
  WiFi.setHostname(HOSTNAME);
  String pass;
  loadWifi(savedSsid, pass);
  if (savedSsid.length() > 0) {
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(savedSsid.c_str(), pass.c_str());
    logPrintf("Conectando em \"%s\"", savedSsid.c_str());
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
      delay(300);
      Serial.print('.');
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
      staMode = true;
      logPrintf("Conectado. Abra http://%s.local ou http://%s\n",
                    HOSTNAME, WiFi.localIP().toString().c_str());
      return;
    }
    logPrintln("Não conectou (senha errada ou rede fora do alcance). Criando a rede própria.");
    WiFi.disconnect(true);
  } else {
    logPrintln("Nenhuma rede salva.");
  }
  // AP + STA: a rede própria funciona e o ESP32 ainda consegue procurar redes.
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASS);
  logPrintf("Rede \"%s\" (senha \"%s\"). Abra http://%s e configure o Wi-Fi no card \"Wi-Fi\".\n",
                AP_SSID, AP_PASS, WiFi.softAPIP().toString().c_str());
}
