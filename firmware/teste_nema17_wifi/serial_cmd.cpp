// serial_cmd.cpp — r g + - l s x z m w f o ?
#include "serial_cmd.h"
#include "log.h"
#include "motion.h"
#include "wifi_net.h"
#include "ota.h"

// ---------------- Serial ----------------
void help() {
  logPrintln("Comandos: r <rpm> | g <graus> | + <graus> | - <graus> | l <graus> | s | x | z | ?");
  logPrintln("Wi-Fi:    w <rede>|<senha> (salva e reinicia) | f (esquece e reinicia)");
  logPrintln("OTA:      o <senha> (define a senha do OTA e reinicia)");
  logPrintln("Motor:    m <micropasso> <redução> (ex.: m 8 1 = motor solto)");
  logPrintf("Rampa %s · batentes %s · ENA %s\n", rampMode ? "curva S" : "trapezoidal",
            limEn ? "ligados" : "desligados", enaUse ? "ligado (GPIO 27)" : "desligado");
  logPrintf("Micropasso 1/%d, redução %.2f:1, %.0f passos por volta da saída\n",
                MICROSTEP, RATIO, STEPS_OUT_REV);
  logPrintf("Velocidade: %.1f rpm na saída (%.0f rpm no motor)\n", (float)rpm, rpm * RATIO);
}

void serialService() {
  static char buf[112];
  static uint8_t n = 0;
  while (Serial.available()) {
    char ch = Serial.read();
    if (ch == '\r') continue;
    if (ch != '\n') { if (n < sizeof(buf) - 1) buf[n++] = ch; continue; }
    buf[n] = 0;
    n = 0;
    String line(buf);
    line.trim();
    if (line.length() == 0) continue;
    char cmd = line.charAt(0);
    float arg = line.substring(1).toFloat();
    if (cmd == 'w') {                     // w nome da rede|senha
      String rest = line.substring(1);
      int bar = rest.indexOf('|');
      String ssid = bar >= 0 ? rest.substring(0, bar) : rest;
      String pass = bar >= 0 ? rest.substring(bar + 1) : "";
      ssid.trim();
      if (ssid.length() == 0) { logPrintln("Uso: w nome da rede|senha"); continue; }
      saveWifi(ssid, pass);
      scheduleRestart();
      continue;
    }
    if (cmd == 'f') { forgetWifi(); scheduleRestart(); continue; }
    if (cmd == 'm') {                     // m <micropasso> <redução>
      int ms = 0; float ratio = 0;
      if (sscanf(line.c_str() + 1, "%d %f", &ms, &ratio) != 2 || !validMotorCfg(ms, ratio)) {
        logPrintln("Uso: m <micropasso> <redução>   ex.: m 8 1 (motor solto) ou m 8 20 (J2)");
        continue;
      }
      saveMotorCfg(ms, ratio);
      sendCmd2(C_CFG, ms, ratio);
      continue;
    }
    if (cmd == 'o') {
      String pass = line.substring(1);
      pass.trim();
      if (pass.length() < 6) { logPrintln("Uso: o <senha> (mínimo 6 caracteres)"); continue; }
      saveOtaPass(pass);
      scheduleRestart();
      continue;
    }
    switch (cmd) {
      case 'r': sendCmd(C_RPM, arg);
                logPrintf("Velocidade: %.1f rpm na saída\n", arg > 0 ? min(arg, RPM_MAX) : (float)rpm);
                break;
      case 'g': sendCmd(C_GOTO, arg);  break;
      case '+': sendCmd(C_MOVE, arg);  break;
      case '-': sendCmd(C_MOVE, -arg); break;
      case 'l': sendCmd(C_LOOP, arg);  break;
      case 's': sendCmd(C_SWEEP, 0);   break;
      case 'x': sendCmd(C_STOP, 0);    break;
      case 'z': sendCmd(C_ZERO, 0);    break;
      default:  help();
    }
  }
}
