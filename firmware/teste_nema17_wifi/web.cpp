// web.cpp — o ESP32 como "cérebro": endpoints em JSON padronizado ({"ok":true} / {"ok":false,"error":"..."})
// e o WebSocket (porta 81), que empurra o status e recebe os comandos. A interface completa roda no
// computador (tools/serve.py); aqui fica só uma página mínima de reserva (parar, status, Wi-Fi).
#include "web.h"
#include "config.h"
#include "log.h"
#include "motion.h"
#include "samples.h"
#include "wifi_net.h"
#include "ota.h"
#include "web_page.h"
#include "ws_server.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>

WebServer server(80);
const char* PAGE_ETAG = "\"" __DATE__ " " __TIME__ "\"";   // identifica esta gravação

// Respostas padronizadas: {"ok":true,...} ou {"ok":false,"error":"..."}
// CORS aberto: a interface vem de outra origem (localhost no Mac ou arquivo local). Os POSTs da
// página usam corpo application/x-www-form-urlencoded, então o navegador não faz pré-voo (OPTIONS).
void sendJson(int code, const String& body) {
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(code, "application/json", body);
}

void sendOk() { sendJson(200, "{\"ok\":true}"); }

void sendErr(int code, const char* msg) { sendJson(code, String("{\"ok\":false,\"error\":\"") + msg + "\"}"); }

// Executa um comando de movimento/config. Devolve nullptr se deu certo, ou a mensagem
// de erro (com o código HTTP em code). Usado pelo /cmd e pelo WebSocket.
const char* runCmd(const String& c, float v, float p2, float p3, float p4, int& code) {
  code = 400;
  CmdType t;
  if      (c == "goto")      t = C_GOTO;
  else if (c == "move")      t = C_MOVE;
  else if (c == "rpm")       t = C_RPM;
  else if (c == "acc")       t = C_ACC;
  else if (c == "ramp")      t = C_RAMP;
  else if (c == "loop")      t = C_LOOP;
  else if (c == "sweep")     t = C_SWEEP;
  else if (c == "accsweep")  t = C_ACC_SWEEP;
  else if (c == "resonance") t = C_RESONANCE;
  else if (c == "backlash")  t = C_BACKLASH;
  else if (c == "drift")     t = C_DRIFT;
  else if (c == "stiction")  t = C_STICTION;
  else if (c == "waypoints") t = C_WAYPOINTS;
  else if (c == "jog")       t = C_JOG;
  else if (c == "stop")      t = C_STOP;
  else if (c == "zero")      t = C_ZERO;
  else if (c == "ena")       t = C_ENA;
  else return "comando desconhecido";
  if (t == C_RESONANCE && limEn) return "ressonância bloqueada com batentes ligados";
  if (t == C_WAYPOINTS && wpCount == 0) return "nenhuma trajetória carregada";
  if (t == C_ENA && !enaUse) return "ENA desligado em Sistema";
  if (t == C_RAMP) saveRampCfg(v > 0 ? 1 : 0, p2 > 0 ? constrain(p2, 1000.0f, 1e7f) : (float)jerk);
  if (!sendCmdN(t, v, p2, p3, p4)) { code = 503; return "fila cheia"; }
  if (t != C_JOG) logPrintf("[web] %s %.2f\n", c.c_str(), v);
  code = 200;
  return nullptr;
}

// /cmd?c=<comando>&v=<p1>&p2=..&p3=..&p4=..
void handleCmd() {
  String c = server.arg("c");
  float v = server.arg("v").toFloat();
  int code;
  const char* err = runCmd(c, v, server.arg("p2").toFloat(), server.arg("p3").toFloat(), server.arg("p4").toFloat(), code);
  if (err) { sendErr(code, err); return; }
  char b[80];
  snprintf(b, sizeof(b), "{\"ok\":true,\"c\":\"%s\",\"v\":%.3f}", c.c_str(), v);
  sendJson(200, b);
}

// Escapa aspas e barras para colocar um texto dentro do JSON.
String jsonStr(const String& s) {
  String o = "\"";
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"' || c == '\\') { o += '\\'; o += c; }
    else if ((uint8_t)c < 0x20) o += ' ';
    else o += c;
  }
  return o + "\"";
}

// Monta o JSON de status com o log e as amostras novas desde os cursores, e avança os cursores.
// Cada conexão (polling HTTP ou WebSocket) tem os seus.
void buildStatus(String& j, uint32_t& lsince, uint32_t& since) {
  char sub[24];
  subModeStr(sub, sizeof(sub));
  char buf[480];
  snprintf(buf, sizeof(buf),
           "{\"pos\":%.2f,\"target\":%.2f,\"rpm\":%.2f,\"moving\":%s,\"mode\":\"%s\",\"subMode\":\"%s\","
           "\"cycles\":%ld,\"sweep\":%.2f,\"stepAcc\":%.0f,\"rpmSet\":%.2f,\"acc\":%.0f,\"ramp\":%d,\"jerk\":%.0f,"
           "\"limEn\":%s,\"limMin\":%.2f,\"limMax\":%.2f,\"enaUse\":%s,\"enaOn\":%s,\"enaInv\":%s,",
           stepsToDeg(pos), stepsToDeg(shownTarget()), vel * 60.0 / STEPS_OUT_REV,
           vel != 0 ? "true" : "false", modeName(mode), sub,
           (long)cycles, mode == M_RESONANCE ? (float)resRpmNow : (float)sweepRpm, (float)sweepAcc,
           (float)rpm, (float)acc, (int)rampMode, (float)jerk,
           limEn ? "true" : "false", (float)limMin, (float)limMax,
           enaUse ? "true" : "false", enaOn ? "true" : "false", enaInv ? "true" : "false");
  j.reserve(1024);                          // menos fragmentação do heap
  j = buf;
  j += "\"net\":\""; j += staMode ? "sta" : "ap"; j += "\"";
  j += ",\"ssid\":" + jsonStr(staMode ? WiFi.SSID() : String(AP_SSID));
  j += ",\"ip\":\"" + (staMode ? WiFi.localIP() : WiFi.softAPIP()).toString() + "\"";
  j += ",\"rssi\":" + String(staMode ? WiFi.RSSI() : 0);
  j += ",\"saved\":" + jsonStr(savedSsid);
  j += String(",\"ota\":") + (otaOn ? "true" : "false");
  j += ",\"micro\":" + String(MICROSTEP) + ",\"ratio\":" + String(RATIO, 2) + ",\"rpmMax\":" + String(RPM_MAX, 1);
  j += ",\"up\":" + String(millis()) + ",\"beat\":" + String(millis() - loopBeat);
  j += ",\"ws\":" + String(wsCount());

  // Linhas novas do log desde lsince
  portENTER_CRITICAL(&logMux);
  uint32_t lnow = logSeq;
  portEXIT_CRITICAL(&logMux);
  uint32_t lfrom = lsince;
  if (lfrom > lnow || lnow - lfrom > LOGN) lfrom = lnow > LOGN ? lnow - LOGN : 0;
  j += ",\"logSeq\":" + String(lnow) + ",\"log\":[";
  for (uint32_t k = lfrom; k < lnow; k++) {
    LogLine L;
    portENTER_CRITICAL(&logMux);
    L = logBuf[k % LOGN];
    portEXIT_CRITICAL(&logMux);
    if (k > lfrom) j += ",";
    j += "[" + String(L.ms) + "," + jsonStr(String(L.txt)) + "]";
  }
  j += "]";
  lsince = lnow;

  // Amostras novas desde since (até o buffer inteiro, ~10 s, para recuperar falhas curtas).
  portENTER_CRITICAL(&ringMux);
  uint32_t seqNow = sampleSeq;
  portEXIT_CRITICAL(&ringMux);
  uint32_t from = since;
  const uint32_t MAXS = RING - 24;          // folga para o timer não sobrescrever o que está sendo lido
  if (from > seqNow || seqNow - from > MAXS) from = seqNow > MAXS ? seqNow - MAXS : 0;
  j.reserve(j.length() + (seqNow - from) * 34 + 16);
  j += ",\"seq\":" + String(seqNow) + ",\"samples\":[";
  for (uint32_t k = from; k < seqNow; k++) {
    Sample s;
    portENTER_CRITICAL(&ringMux);
    s = ring[k % RING];
    portEXIT_CRITICAL(&ringMux);
    char b[64];
    snprintf(b, sizeof(b), "%s[%lu,%.2f,%.2f,%.2f]", k > from ? "," : "",
             (unsigned long)s.t, s.pos, s.tgt, s.rpm);
    j += b;
  }
  j += "]}";
  since = seqNow;
}

void handleStatus() {
  uint32_t lsince = (uint32_t)server.arg("lsince").toInt();
  uint32_t since = (uint32_t)server.arg("since").toInt();
  String j;
  buildStatus(j, lsince, since);
  sendJson(200, j);
}

// ---------- WebSocket ----------
// Página → ESP32:  "s|since=N&lsince=M"          de onde continuar (a página já tem até ali)
//                  "<id>|c=goto&v=90&p2=..."      comando; resposta {"re":<id>,"ok":true,...}
// ESP32 → página a cada 100 ms: o mesmo JSON do /status, só com o que é novo para essa conexão.
struct WsCursor { uint32_t lsince = 0, since = 0; bool ready = false; };
WsCursor wsCur[WS_MAX_CLIENTS];
const uint32_t WS_PUSH_MS = 100;

// Lê o valor de uma chave numa query "a=1&b=2" (sem decodificar %xx: só números e nomes simples).
String queryArg(const char* q, const char* key) {
  size_t kl = strlen(key);
  for (const char* p = q; *p;) {
    const char* amp = strchr(p, '&');
    const char* end = amp ? amp : p + strlen(p);
    if ((size_t)(end - p) > kl && strncmp(p, key, kl) == 0 && p[kl] == '=') {
      String v;
      for (const char* x = p + kl + 1; x < end; x++) v += *x;
      return v;
    }
    p = amp ? amp + 1 : end;
  }
  return String();
}

void onWs(uint8_t c, WsEvent ev, const char* msg, size_t /*len*/) {
  if (ev == WS_OPEN) { wsCur[c] = WsCursor(); logPrintf("[ws] página conectada (%u)\n", (unsigned)wsCount()); return; }
  if (ev == WS_CLOSE) return;
  const char* bar = strchr(msg, '|');
  if (!bar) return;
  const char* q = bar + 1;
  if (msg[0] == 's' && bar == msg + 1) {     // sincronização: só depois dela o ESP32 começa a empurrar
    wsCur[c].since = (uint32_t)queryArg(q, "since").toInt();
    wsCur[c].lsince = (uint32_t)queryArg(q, "lsince").toInt();
    wsCur[c].ready = true;
    return;
  }
  long id = atol(msg);
  String cmd = queryArg(q, "c");
  float v = queryArg(q, "v").toFloat();
  int code;
  const char* err = runCmd(cmd, v, queryArg(q, "p2").toFloat(), queryArg(q, "p3").toFloat(), queryArg(q, "p4").toFloat(), code);
  char b[160];
  int n = err ? snprintf(b, sizeof(b), "{\"re\":%ld,\"ok\":false,\"error\":\"%s\"}", id, err)
              : snprintf(b, sizeof(b), "{\"re\":%ld,\"ok\":true,\"c\":\"%s\",\"v\":%.3f}", id, cmd.c_str(), v);
  wsSendText(c, b, n);
}

void wsPush() {
  static uint32_t last = 0;
  if (millis() - last < WS_PUSH_MS) return;
  last = millis();
  for (uint8_t c = 0; c < WS_MAX_CLIENTS; c++) {
    if (!wsConnected(c) || !wsCur[c].ready) continue;
    String j;
    buildStatus(j, wsCur[c].lsince, wsCur[c].since);
    wsSendText(c, j.c_str(), j.length());
  }
}

// Busca de redes sem travar: a primeira chamada começa, as seguintes pegam o resultado.
// ?fresh=1 descarta o resultado anterior e começa outra busca.
void handleScan() {
  int n = WiFi.scanComplete();
  if (server.hasArg("fresh") && n >= 0) { WiFi.scanDelete(); n = WIFI_SCAN_FAILED; }
  if (n == WIFI_SCAN_FAILED) { WiFi.scanNetworks(true); sendJson(200, "{\"done\":false}"); return; }
  if (n == WIFI_SCAN_RUNNING) { sendJson(200, "{\"done\":false}"); return; }
  // deduplica mantendo o sinal mais forte (redes mesh), ordena por sinal, no máximo 25
  int idx[64], m = 0;
  for (int i = 0; i < n && i < 64; i++) {
    String s = WiFi.SSID(i);
    if (s.length() == 0) continue;
    int found = -1;
    for (int k = 0; k < m; k++) if (WiFi.SSID(idx[k]) == s) { found = k; break; }
    if (found < 0) idx[m++] = i;
    else if (WiFi.RSSI(i) > WiFi.RSSI(idx[found])) idx[found] = i;
  }
  for (int a = 1; a < m; a++) {
    int x = idx[a], r = WiFi.RSSI(x), b = a - 1;
    while (b >= 0 && WiFi.RSSI(idx[b]) < r) { idx[b + 1] = idx[b]; b--; }
    idx[b + 1] = x;
  }
  if (m > 25) m = 25;
  String j;
  j.reserve(40 + m * 80);
  j = "{\"done\":true,\"nets\":[";
  for (int k = 0; k < m; k++) {
    int i = idx[k];
    if (k) j += ",";
    j += "{\"ssid\":";
    j += jsonStr(WiFi.SSID(i));
    j += ",\"rssi\":";
    j += String(WiFi.RSSI(i));
    j += ",\"enc\":";
    j += (WiFi.encryptionType(i) != WIFI_AUTH_OPEN) ? "true" : "false";
    j += ",\"ch\":";
    j += String(WiFi.channel(i));
    j += "}";
  }
  j += "]}";
  WiFi.scanDelete();
  sendJson(200, j);
}

void handleWifiSave() {
  String ssid = server.arg("ssid");
  ssid.trim();
  String pass = server.arg("pass");
  if (ssid.length() == 0 || ssid.length() > 32 || pass.length() > 64) {
    sendErr(400, "rede ou senha inválida");
    return;
  }
  saveWifi(ssid, pass);
  sendOk();
  scheduleRestart();
}

void handleWifiForget() {
  forgetWifi();
  sendOk();
  scheduleRestart();
}

void handleOtaSave() {
  String pass = server.arg("pass");
  if (pass.length() < 6 || pass.length() > 64) {
    sendErr(400, "a senha precisa ter de 6 a 64 caracteres");
    return;
  }
  saveOtaPass(pass);
  sendOk();
  scheduleRestart();
}

void handleMotorSave() {
  int ms = server.arg("ms").toInt();
  float ratio = server.arg("ratio").toFloat();
  if (!validMotorCfg(ms, ratio)) {
    sendErr(400, "micropasso deve ser 1, 2, 4 … 256 e redução entre 0,1 e 1000");
    return;
  }
  saveMotorCfg(ms, ratio);
  if (!sendCmd2(C_CFG, ms, ratio)) { sendErr(503, "fila cheia"); return; }
  sendOk();
}

// POST /waypoints  pts=graus,rpm,ms;graus,rpm,ms;...  loop=0|1   (1 a 8 pontos; rpm 0 = velocidade atual)
void handleWaypoints() {
  String pts = server.arg("pts");
  bool lp = server.arg("loop") == "1";
  Waypoint tmp[8];
  int n = 0, start = 0;
  while (start < (int)pts.length()) {
    int e = pts.indexOf(';', start);
    if (e < 0) e = pts.length();
    String part = pts.substring(start, e);
    start = e + 1;
    part.trim();
    if (part.length() == 0) continue;
    float d = 0, r = 0; unsigned long w = 0;
    if (sscanf(part.c_str(), "%f,%f,%lu", &d, &r, &w) != 3) { sendErr(400, "ponto inválido"); return; }
    if (n >= 8) { sendErr(400, "no máximo 8 pontos"); return; }
    if (limEn && (d < limMin || d > limMax)) { sendErr(400, "ponto fora dos batentes"); return; }
    if (r <= 0) r = rpm;
    tmp[n].deg = d; tmp[n].rpm = r; tmp[n].dwellMs = (uint32_t)w;
    n++;
  }
  if (n < 1) { sendErr(400, "nenhum ponto"); return; }
  portENTER_CRITICAL(&wpMux);
  for (int i = 0; i < n; i++) wpBuf[i] = tmp[i];
  wpCount = n; wpLoop = lp;
  portEXIT_CRITICAL(&wpMux);
  if (!sendCmd(C_WAYPOINTS, 0)) { sendErr(503, "fila cheia"); return; }
  logPrintf("[web] trajetória com %d pontos%s\n", n, lp ? " em loop" : "");
  sendJson(200, String("{\"ok\":true,\"n\":") + n + "}");
}

// POST /safety  limEn, limMin, limMax, enaUse, enaInv   (salva na NVS e aplica)
void handleSafety() {
  bool le = server.arg("limEn") == "1";
  float lo = server.arg("limMin").toFloat(), hi = server.arg("limMax").toFloat();
  bool eu = server.arg("enaUse") == "1", ei = server.arg("enaInv") == "1";
  if (le && !(lo < hi)) { sendErr(400, "mínimo precisa ser menor que o máximo"); return; }
  if (!(lo < hi)) { lo = -120.0; hi = 120.0; }
  saveSafetyCfg(le, lo, hi, eu, ei);
  if (!sendCmdN(C_LIMITS, le ? 1 : 0, lo, hi, (eu ? 1 : 0) | (ei ? 2 : 0))) { sendErr(503, "fila cheia"); return; }
  sendOk();
}

void webTask(void*) {
  for (;;) {
    server.handleClient();
    wsLoop();
    wsPush();
    if (otaOn) ArduinoOTA.handle();
    if (restartAt && (int32_t)(millis() - restartAt) >= 0) ESP.restart();
    vTaskDelay(1);
  }
}

// Rotas do servidor e tarefa web no núcleo 0 (junto com o Wi-Fi), para não atrapalhar os passos.
void webBegin() {
  // Página de reserva (ou a completa, se gerada com build_web.py --embutir), em gzip e com ETag: muda a cada gravação do firmware. Se o navegador
  // já tem a versão atual, o ESP32 responde "304" (sem corpo) em vez de mandar a página.
  server.on("/", []() {
    if (server.header("If-None-Match") == PAGE_ETAG) { server.send(304); return; }
    server.sendHeader("ETag", PAGE_ETAG);
    server.sendHeader("Cache-Control", "no-cache");   // sempre confere, mas reaproveita se não mudou
    server.sendHeader("Content-Encoding", "gzip");
    server.send_P(200, "text/html", (const char*)PAGE_GZ, PAGE_GZ_LEN);
  });
  static const char* hdrs[] = { "If-None-Match" };
  server.collectHeaders(hdrs, 1);
  server.on("/cmd", handleCmd);
  server.on("/status", handleStatus);
  server.on("/scan", handleScan);
  server.on("/wifi", HTTP_POST, handleWifiSave);
  server.on("/wifi/forget", HTTP_POST, handleWifiForget);
  server.on("/ota", HTTP_POST, handleOtaSave);
  server.on("/motor", HTTP_POST, handleMotorSave);
  server.on("/waypoints", HTTP_POST, handleWaypoints);
  server.on("/safety", HTTP_POST, handleSafety);
  server.begin();
  wsBegin(81, onWs);
  xTaskCreatePinnedToCore(webTask, "web", 8192, nullptr, 1, nullptr, 0);
}
