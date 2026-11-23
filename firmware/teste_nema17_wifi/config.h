// config.h — pinos e constantes fixas do nemabot (teste do J2).
#pragma once
#include <Arduino.h>

// Wi-Fi e OTA (as senhas ficam na memória do ESP32, nunca aqui)
constexpr const char* PREFS_NS = "wifi";   // rede salva
constexpr const char* OTA_NS   = "ota";    // senha do OTA ("Esquecer rede" não apaga)
constexpr const char* MOTOR_NS = "motor";  // micropasso, redução, rampa, batentes, ENA
constexpr const char* AP_SSID  = "nemabot";      // rede própria do ESP32
constexpr const char* AP_PASS  = "nemabot123";   // mínimo 8 caracteres
constexpr const char* HOSTNAME = "nemabot";      // http://nemabot.local

// Pinos (DevKit de 30 pinos)
constexpr int PIN_STEP = 25;   // TB6600: PUL+   · TMC2209: STEP
constexpr int PIN_DIR  = 26;   // TB6600: DIR+   · TMC2209: DIR
constexpr int PIN_ENA  = 27;   // TB6600: ENA+   · TMC2209: EN  (opcional, ligado em Sistema)

// Limites do movimento
constexpr float MOTOR_RPM_MAX = 600.0;     // limite no eixo do motor
constexpr float V_START = 200.0;           // velocidade de partida, passos/s
constexpr float ACC_MAX = 50000.0;         // limite de aceleração, passos/s²
constexpr long  BIG = 20000000L;           // alvo "infinito" do jog e da ressonância
