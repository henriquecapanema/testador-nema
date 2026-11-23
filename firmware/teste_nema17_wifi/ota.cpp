// ota.cpp — OTA só liga com senha definida; para o motor antes de gravar.
#include "ota.h"
#include "config.h"
#include "log.h"
#include "motion.h"
#include <ArduinoOTA.h>
#include <Preferences.h>

bool otaOn = false;               // true = OTA ligado (tem senha salva)

String loadOtaPass() {
  Preferences p;
  p.begin(OTA_NS, true);
  String s = p.getString("pass", "");
  p.end();
  return s;
}

void saveOtaPass(const String& pass) {
  Preferences p;
  p.begin(OTA_NS, false);
  p.putString("pass", pass);
  p.end();
  logPrintln("Senha do OTA salva. Reiniciando…");
}

void setupOta() {
  String pass = loadOtaPass();
  if (pass.length() == 0) {
    logPrintln("OTA desligado: defina uma senha na página (card \"Atualização pelo Wi-Fi\").");
    return;
  }
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword(pass.c_str());
  ArduinoOTA.onStart([]() {
    logPrintln("OTA: recebendo código novo. Parando o motor…");
    sendCmd(C_STOP, 0);
    uint32_t t0 = millis();                      // espera a rampa de parada (até 3 s)
    while ((vel != 0 || uxQueueMessagesWaiting(cmdQueue)) && millis() - t0 < 3000) vTaskDelay(10);
  });
  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {
    static int last = -1;
    int pct = done * 100 / total;
    if (pct / 10 != last) { last = pct / 10; logPrintf("OTA: %d%%\n", pct); }
  });
  ArduinoOTA.onEnd([]() { logPrintln("OTA: pronto, reiniciando."); });
  ArduinoOTA.onError([](ota_error_t e) {
    const char* m = e == OTA_AUTH_ERROR ? "senha errada" : e == OTA_BEGIN_ERROR ? "sem espaço (Partition Scheme sem OTA?)"
                  : e == OTA_CONNECT_ERROR ? "não conectou de volta no computador (firewall?)"
                  : e == OTA_RECEIVE_ERROR ? "falha ao receber" : "falha ao finalizar";
    logPrintf("OTA: erro, %s\n", m);
  });
  ArduinoOTA.begin();                            // também inicia o mDNS (nemabot.local)
  otaOn = true;
  logPrintln("OTA ligado: no Arduino IDE escolha a porta de rede \"nemabot\".");
}
