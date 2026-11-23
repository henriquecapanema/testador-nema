// ws_server.h — servidor WebSocket mínimo (RFC 6455), sem biblioteca externa.
// A página abre uma conexão só e o ESP32 empurra o status por ela; os comandos
// voltam pelo mesmo caminho. Feito para mensagens pequenas de texto.
#pragma once
#include <Arduino.h>

constexpr uint8_t WS_MAX_CLIENTS = 4;

enum WsEvent : uint8_t { WS_OPEN, WS_CLOSE, WS_TEXT };
typedef void (*WsHandler)(uint8_t client, WsEvent ev, const char* msg, size_t len);

void wsBegin(uint16_t port, WsHandler handler);
void wsLoop();                                         // aceita conexões e lê mensagens; chamar sempre
bool wsConnected(uint8_t client);
uint8_t wsCount();
bool wsSendText(uint8_t client, const char* s, size_t len);   // false = conexão caiu e foi fechada

// Expostos para os testes
void wsSha1(const uint8_t* data, size_t n, uint8_t out[20]);
size_t wsBase64(const uint8_t* data, size_t n, char* out);
void wsAcceptKey(const char* key, char out[29]);
