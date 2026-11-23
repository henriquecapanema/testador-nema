// ws_server.cpp — WebSocket mínimo sobre WiFiServer. Só o necessário para a página:
// handshake, quadros de texto pequenos (até ~1 KB recebidos), ping/pong e fechamento.
#include "ws_server.h"
#include <WiFi.h>
#include <errno.h>
#ifdef ESP_PLATFORM
#include <lwip/sockets.h>
#else
#include <sys/socket.h>
#endif

// Lemos e escrevemos direto no socket (fd), sem WiFiClient::connected()/available()/write():
// no ESP32 o connected() decide pelo errno global, que pode ter sobrado de outro socket
// (um pedido HTTP que acabou de fechar), e derrubava o WebSocket à toa.

namespace {

struct Slot {
  WiFiClient c;
  uint8_t state = 0;          // 0 livre, 1 aguardando handshake, 2 aberta
  uint32_t t0 = 0;            // quando conectou (para o prazo do handshake)
  size_t n = 0;               // bytes no buffer de entrada
  uint8_t buf[1024];
};

WiFiServer* srv = nullptr;
WsHandler onEvent = nullptr;
Slot slots[WS_MAX_CLIENTS];
const uint32_t HANDSHAKE_MS = 3000;

// Manda tudo (o socket é bloqueante; send devolve quanto foi). false = erro na conexão.
bool sendAll(Slot& s, const uint8_t* p, size_t len) {
  int fd = s.c.fd();
  uint32_t t0 = millis();
  while (len) {
    errno = 0;
    int r = send(fd, p, len, 0);
    if (r > 0) { p += r; len -= r; continue; }
    if (r < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) && millis() - t0 < 2000) { delay(1); continue; }
    return false;
  }
  return true;
}

void drop(uint8_t i, bool sendClose) {
  Slot& s = slots[i];
  if (s.state == 0) return;
  bool wasOpen = s.state == 2;
  if (sendClose && wasOpen) {
    const uint8_t fr[2] = { 0x88, 0x00 };
    send(s.c.fd(), fr, 2, MSG_DONTWAIT);
  }
  s.c.stop();
  s.state = 0;
  s.n = 0;
  if (wasOpen && onEvent) onEvent(i, WS_CLOSE, nullptr, 0);
}

bool sendFrame(uint8_t i, uint8_t op, const uint8_t* p, size_t len) {
  Slot& s = slots[i];
  if (s.state != 2) return false;
  uint8_t h[10];
  size_t hn = 0;
  h[hn++] = 0x80 | op;                         // FIN + opcode (servidor não mascara)
  if (len < 126) h[hn++] = (uint8_t)len;
  else if (len < 65536) { h[hn++] = 126; h[hn++] = len >> 8; h[hn++] = len & 0xFF; }
  else { h[hn++] = 127; for (int k = 7; k >= 0; k--) h[hn++] = (k < 4) ? (uint8_t)(len >> (8 * k)) : 0; }
  if (!sendAll(s, h, hn) || (len && !sendAll(s, p, len))) { drop(i, false); return false; }
  return true;
}

// Procura "nome:" no cabeçalho sem diferenciar maiúsculas; devolve o valor (sem espaços) em out.
bool headerValue(const char* req, const char* name, char* out, size_t outLen) {
  size_t nl = strlen(name);
  for (const char* p = req; *p; p++) {
    if ((p == req || p[-1] == '\n') && strncasecmp(p, name, nl) == 0 && p[nl] == ':') {
      p += nl + 1;
      while (*p == ' ' || *p == '\t') p++;
      size_t k = 0;
      while (*p && *p != '\r' && *p != '\n' && k + 1 < outLen) out[k++] = *p++;
      while (k && out[k - 1] == ' ') k--;
      out[k] = 0;
      return k > 0;
    }
  }
  return false;
}

void handshake(uint8_t i) {
  Slot& s = slots[i];
  s.buf[s.n] = 0;
  const char* req = (const char*)s.buf;
  if (!strstr(req, "\r\n\r\n")) {
    if (s.n >= sizeof(s.buf) - 1 || millis() - s.t0 > HANDSHAKE_MS) drop(i, false);
    return;
  }
  char key[64];
  if (!headerValue(req, "Sec-WebSocket-Key", key, sizeof(key))) {
    static const char bad[] = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";
    sendAll(s, (const uint8_t*)bad, sizeof(bad) - 1);
    drop(i, false);
    return;
  }
  char acc[29];
  wsAcceptKey(key, acc);
  char resp[160];
  int rn = snprintf(resp, sizeof(resp),
                    "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
                    "Sec-WebSocket-Accept: %s\r\n\r\n", acc);
  if (!sendAll(s, (const uint8_t*)resp, rn)) { drop(i, false); return; }
  s.state = 2;
  s.n = 0;                                     // o navegador só manda quadros depois da resposta
  if (onEvent) onEvent(i, WS_OPEN, nullptr, 0);
}

// Processa os quadros completos que estão no buffer.
void readFrames(uint8_t i) {
  Slot& s = slots[i];
  while (s.state == 2 && s.n >= 2) {
    uint8_t* b = s.buf;
    bool fin = b[0] & 0x80;
    uint8_t op = b[0] & 0x0F;
    bool masked = b[1] & 0x80;
    size_t len = b[1] & 0x7F, hdr = 2;
    if (len == 126) { if (s.n < 4) return; len = ((size_t)b[2] << 8) | b[3]; hdr = 4; }
    else if (len == 127) { drop(i, true); return; }          // grande demais para nós
    if (!masked) { drop(i, true); return; }                   // cliente sempre mascara (RFC 6455 5.1)
    hdr += 4;
    if (hdr + len >= sizeof(s.buf)) { drop(i, true); return; }
    if (s.n < hdr + len) return;                              // quadro ainda incompleto
    uint8_t* m = b + hdr - 4;
    uint8_t* p = b + hdr;
    for (size_t k = 0; k < len; k++) p[k] ^= m[k & 3];
    size_t total = hdr + len;
    uint8_t keep = p[len];
    p[len] = 0;                                               // termina o texto para o handler
    if (op == 0x8) { drop(i, true); return; }
    if (op == 0x9) sendFrame(i, 0xA, p, len);                 // ping → pong
    else if (op == 0x1 && fin && onEvent) onEvent(i, WS_TEXT, (const char*)p, len);
    // binário, pong e fragmentos são ignorados
    if (s.state != 2) return;
    p[len] = keep;
    memmove(b, b + total, s.n - total);
    s.n -= total;
  }
}

}  // namespace

void wsBegin(uint16_t port, WsHandler handler) {
  onEvent = handler;
  srv = new WiFiServer(port);
  srv->begin();
  srv->setNoDelay(true);
}

void wsLoop() {
  if (!srv) return;
  if (srv->hasClient()) {
    WiFiClient nc = srv->available();
    int slot = -1;
    for (int i = 0; i < WS_MAX_CLIENTS; i++) if (slots[i].state == 0) { slot = i; break; }
    if (slot < 0) nc.stop();
    else {
      Slot& s = slots[slot];
      s.c = nc;
      s.c.setNoDelay(true);
      s.state = 1;
      s.n = 0;
      s.t0 = millis();
    }
  }
  for (uint8_t i = 0; i < WS_MAX_CLIENTS; i++) {
    Slot& s = slots[i];
    if (s.state == 0) continue;
    size_t room = sizeof(s.buf) - 1 - s.n;
    if (room == 0) { drop(i, true); continue; }
    errno = 0;
    int got = recv(s.c.fd(), s.buf + s.n, room, MSG_DONTWAIT);
    if (got > 0) s.n += got;
    else if (got == 0) { drop(i, false); continue; }                        // o outro lado fechou
    else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) { drop(i, false); continue; }
    if (s.state == 1) handshake(i);
    else readFrames(i);
  }
}

bool wsConnected(uint8_t client) { return client < WS_MAX_CLIENTS && slots[client].state == 2; }

uint8_t wsCount() {
  uint8_t k = 0;
  for (auto& s : slots) k += s.state == 2;
  return k;
}

bool wsSendText(uint8_t client, const char* s, size_t len) {
  if (client >= WS_MAX_CLIENTS) return false;
  return sendFrame(client, 0x1, (const uint8_t*)s, len);
}

// ---------- SHA-1 e base64, só para o handshake ----------
static inline uint32_t rol(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }

void wsSha1(const uint8_t* data, size_t n, uint8_t out[20]) {
  uint32_t h[5] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };
  uint64_t bits = (uint64_t)n * 8;
  size_t total = ((n + 8) / 64 + 1) * 64;
  uint8_t blk[64];
  for (size_t off = 0; off < total; off += 64) {
    for (size_t k = 0; k < 64; k++) {
      size_t idx = off + k;
      if (idx < n) blk[k] = data[idx];
      else if (idx == n) blk[k] = 0x80;
      else if (idx >= total - 8) blk[k] = (uint8_t)(bits >> (8 * (total - 1 - idx)));
      else blk[k] = 0;
    }
    uint32_t w[80];
    for (int t = 0; t < 16; t++) w[t] = (uint32_t)blk[4 * t] << 24 | (uint32_t)blk[4 * t + 1] << 16 | (uint32_t)blk[4 * t + 2] << 8 | blk[4 * t + 3];
    for (int t = 16; t < 80; t++) w[t] = rol(w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16], 1);
    uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
    for (int t = 0; t < 80; t++) {
      uint32_t f, k;
      if (t < 20)      { f = (b & c) | (~b & d);          k = 0x5A827999; }
      else if (t < 40) { f = b ^ c ^ d;                   k = 0x6ED9EBA1; }
      else if (t < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
      else             { f = b ^ c ^ d;                   k = 0xCA62C1D6; }
      uint32_t tmp = rol(a, 5) + f + e + k + w[t];
      e = d; d = c; c = rol(b, 30); b = a; a = tmp;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
  }
  for (int i = 0; i < 5; i++) for (int k = 0; k < 4; k++) out[4 * i + k] = h[i] >> (24 - 8 * k);
}

size_t wsBase64(const uint8_t* d, size_t n, char* out) {
  static const char T[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  size_t o = 0;
  for (size_t i = 0; i < n; i += 3) {
    uint32_t v = (uint32_t)d[i] << 16 | (i + 1 < n ? (uint32_t)d[i + 1] << 8 : 0) | (i + 2 < n ? d[i + 2] : 0);
    out[o++] = T[(v >> 18) & 63];
    out[o++] = T[(v >> 12) & 63];
    out[o++] = i + 1 < n ? T[(v >> 6) & 63] : '=';
    out[o++] = i + 2 < n ? T[v & 63] : '=';
  }
  out[o] = 0;
  return o;
}

void wsAcceptKey(const char* key, char out[29]) {
  static const char GUID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  char cat[100];
  size_t kl = strlen(key);
  if (kl > sizeof(cat) - sizeof(GUID)) kl = sizeof(cat) - sizeof(GUID);
  memcpy(cat, key, kl);
  memcpy(cat + kl, GUID, sizeof(GUID) - 1);
  uint8_t dig[20];
  wsSha1((const uint8_t*)cat, kl + sizeof(GUID) - 1, dig);
  wsBase64(dig, 20, out);
}
