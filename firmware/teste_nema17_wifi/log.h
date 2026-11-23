// log.h — tudo que vai para o Monitor Serial também fica guardado (últimas 64 linhas)
// e aparece no card "Log" da página.
#pragma once
#include <Arduino.h>

struct LogLine { uint32_t ms; char txt[100]; };
constexpr uint32_t LOGN = 64;
extern LogLine logBuf[LOGN];
extern volatile uint32_t logSeq;
extern portMUX_TYPE logMux;

void logPush(const char* s);
void logPrintf(const char* fmt, ...);
void logPrintln(const char* s);
