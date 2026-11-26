// teste_nema17_wifi.ino
// Mesmo teste do teste_nema17.ino (NEMA 17 + cicloidal do J2, 20:1), agora com
// uma página web pelo Wi-Fi do ESP32. O Monitor Serial (115200) continua funcionando.
//
// Wi-Fi (nada de senha no código):
//   A rede de casa fica salva na memória interna do ESP32 (NVS), que não se apaga
//   ao gravar outro sketch. Sem rede salva, ou se ela não conectar em 15 s, o ESP32
//   cria a rede própria "nemabot" (senha "nemabot123").
//   Primeira vez: conecte na rede "nemabot", abra http://192.168.4.1, e no card
//   "Wi-Fi" escolha a sua rede, digite a senha e salve. Ele reinicia já na rede de
//   casa: volte o Mac para a rede de casa e abra http://nemabot.local
//   (ou o IP que aparece no Monitor Serial).
//   Pelo Serial também dá: "w nome da rede|senha" salva, "f" esquece a rede.
//   Só a rede de 2,4 GHz funciona no ESP32.
//
// Atualizar o código pelo Wi-Fi (OTA):
//   1. Grave esta versão uma vez pelo cabo.
//   2. Na página, card "Atualização pelo Wi-Fi", defina uma senha (fica na memória do
//      ESP32, não no código). Sem senha o OTA fica desligado.
//   3. No Arduino IDE: Ferramentas -> Porta -> "nemabot at <IP>" (porta de rede) e
//      Upload normal. O IDE pede a senha. Na primeira vez o macOS pode perguntar se
//      permite conexões: aceite.
//   Partition Scheme precisa ter OTA (o padrão "Default 4MB with spiffs" serve).
//   Se uma versão nova travar o Wi-Fi, o OTA some junto: aí é pelo cabo de novo.
//   Pelo Serial: "o <senha>" define a senha e reinicia.
//
// Ligações (a aba "Ligações" da página mostra o diagrama do TB6600, do TMC2209 e do SERVO42D):
//   GPIO 25 -> STEP (TB6600: PUL+)    GPIO 26 -> DIR (TB6600: DIR+)
//   GPIO 27 -> ENA (TB6600: ENA+ / TMC2209: EN), opcional, ligado em Sistema
//   GND do ESP32 -> PUL-, DIR-, ENA- do TB6600 (ou GND do TMC2209), e GND da fonte
// Driver em micropasso 1/8 (TB6600: S1 OFF, S2 ON, S3 OFF).
// MKS SERVO42D no modo de pulso (CR_vFOC): STP/DIR/EN nos mesmos GPIOs e COM no 3V3.
//
// Página: abas Controle (absoluto, relativo, jog, rampa trapezoidal ou curva S),
// Testes (8 rotinas: endurance, limite de rpm, limite de aceleração, ressonância,
// folga, repetibilidade, atrito estático e trajetória; tecla M marca falha),
// Ligações (diagrama) e Sistema (Wi-Fi, motor, segurança/ENA e OTA).
// Segurança: batentes virtuais limitam todo alvo; o jog para sozinho se ficar
// 450 ms sem renovação (Wi-Fi caiu); soltar o torque sempre para com rampa antes.
//
// Ângulos e rpm são sempre NA SAÍDA da redução. A posição 0° é onde o eixo estava
// ao ligar (ou onde você apertou "Zerar posição").
//
// Comandos no Monitor Serial ("Nova linha" como fim de linha):
//   r <rpm>  velocidade      g <graus> ir para ângulo absoluto
//   + <graus> / - <graus>    gira relativo
//   l <graus> loop ida e volta   s varredura de velocidade   x parar   z zerar   ? ajuda
//   m <micropasso> <redução>  configura o motor e reinicia (ex.: "m 8 1" = motor solto)
//   w <rede>|<senha> salva o Wi-Fi e reinicia    f esquece o Wi-Fi e reinicia
//
// Como funciona por dentro: o motor é tocado no loop() (núcleo 1), um passo por vez,
// sem travar. A cada passo ele recalcula a velocidade em direção ao alvo, com rampa.
// Por isso o alvo pode mudar no meio do movimento (é o que a barra de -360 a 360 faz).
// O servidor web roda numa tarefa separada no núcleo 0, junto com o Wi-Fi, e só manda
// comandos para o motor por uma fila. A página abre um WebSocket (porta 81): o ESP32
// empurra o status a cada 100 ms e recebe os comandos por ele. Se o WebSocket não
// conectar, a página volta a perguntar por HTTP (/status), como antes.
//
// Arquivos (cada um vira uma aba no Arduino IDE):
//   config.h            pinos e constantes
//   motion.h/.cpp       motor: rampas, rotinas de teste, batentes, ENA, fila de comandos
//   samples.h/.cpp      amostras do gráfico (100 Hz)
//   log.h/.cpp          log no Serial e na página
//   wifi_net.h/.cpp     Wi-Fi salvo e rede própria
//   ota.h/.cpp          atualização pelo Wi-Fi
//   web.h/.cpp          servidor e endpoints
//   ws_server.h/.cpp    WebSocket mínimo (sem biblioteca externa)
//   web_page.h          a página (HTML/CSS/JS em gzip), gerada por tools/build_web.py
//   serial_cmd.h/.cpp   comandos do Monitor Serial

#include <WiFi.h>
#include <ESPmDNS.h>
#include "config.h"
#include "log.h"
#include "motion.h"
#include "samples.h"
#include "wifi_net.h"
#include "ota.h"
#include "web.h"
#include "serial_cmd.h"

void setup() {
  Serial.begin(115200);
  loadMotorCfg();
  if (enaUse) { pinMode(PIN_ENA, OUTPUT); setEna(true); }   // torque ligado ao iniciar
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  digitalWrite(PIN_STEP, LOW);
  delay(500);

  cmdQueue = xQueueCreate(32, sizeof(Cmd));
  setupWifi();
  setupOta();
  if (otaOn || MDNS.begin(HOSTNAME)) MDNS.addService("http", "tcp", 80);

  webBegin();
  startSampling();

  help();
}

void loop() {
  loopBeat = millis();
  motionService();
  if (vel == 0 || (pos & 0xFF) == 0) serialService();
}
