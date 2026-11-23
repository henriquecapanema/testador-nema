// log.cpp — log no Serial e num buffer circular lido pela página.
#include "log.h"
#include <stdarg.h>

LogLine logBuf[LOGN];
volatile uint32_t logSeq = 0;
portMUX_TYPE logMux = portMUX_INITIALIZER_UNLOCKED;


void logPush(const char* s) {
  LogLine L;
  L.ms = millis();
  strlcpy(L.txt, s, sizeof(L.txt));
  size_t n = strlen(L.txt);
  while (n && (L.txt[n - 1] == '\n' || L.txt[n - 1] == '\r')) L.txt[--n] = 0;
  portENTER_CRITICAL(&logMux);
  logBuf[logSeq % LOGN] = L;
  logSeq = logSeq + 1;
  portEXIT_CRITICAL(&logMux);
}

void logPrintf(const char* fmt, ...) {
  char b[160];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(b, sizeof(b), fmt, ap);
  va_end(ap);
  Serial.print(b);
  logPush(b);
}

void logPrintln(const char* s) {
  Serial.println(s);
  logPush(s);
}
