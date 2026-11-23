#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cmath>
#include <string>
#include <functional>
#include <algorithm>
using std::min; using std::max;
#define PROGMEM
#define F(x) (x)
typedef const char* PGM_P;
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define constrain(a,lo,hi) ((a)<(lo)?(lo):((a)>(hi)?(hi):(a)))
uint32_t millis(); uint32_t micros();
inline void delay(uint32_t){} inline void delayMicroseconds(uint32_t){}
inline void digitalWrite(int,int){} inline void pinMode(int,int){}
size_t strlcpy(char*,const char*,size_t);
class String {
  std::string s;
public:
  String(){} String(const char* c):s(c?c:""){} String(const std::string& x):s(x){}
  String(char c):s(1,c){}
  String(int v){s=std::to_string(v);} String(unsigned v){s=std::to_string(v);} String(long v){s=std::to_string(v);} String(unsigned long v){s=std::to_string(v);}
  String(float v,unsigned char d=2){char b[32];snprintf(b,32,"%.*f",d,v);s=b;} String(double v,unsigned char d=2){char b[32];snprintf(b,32,"%.*f",d,v);s=b;}
  size_t length()const{return s.size();} const char* c_str()const{return s.c_str();}
  char operator[](size_t i)const{return s[i];} char charAt(size_t i)const{return s[i];}
  String substring(size_t a)const{return String(s.substr(std::min(a,s.size())));}
  String substring(size_t a,size_t b)const{return String(s.substr(a,b-a));}
  int indexOf(char c,size_t from=0)const{auto p=s.find(c,from);return p==std::string::npos?-1:(int)p;}
  void trim(){} long toInt()const{return atol(s.c_str());} float toFloat()const{return atof(s.c_str());}
  bool reserve(size_t n){s.reserve(n);return true;}
  String& operator+=(const String& o){s+=o.s;return *this;} String& operator+=(const char* o){s+=o;return *this;} String& operator+=(char c){s+=c;return *this;}
  bool operator==(const String& o)const{return s==o.s;} bool operator==(const char* o)const{return s==o;}
  bool operator!=(const char* o)const{return s!=o;}
  friend String operator+(const String& a,const String& b){return String(a.s+b.s);}
  friend String operator+(const String& a,const char* b){return String(a.s+b);}
  friend String operator+(const char* a,const String& b){return String(std::string(a)+b.s);}
  friend String operator+(const String& a,int b){return String(a.s+std::to_string(b));}
};
struct SerialT{void begin(int){} int available(){return 0;} int read(){return 0;} void print(const char*){} void print(char){} void println(const char* =""){} int printf(const char*,...){return 0;}};
extern SerialT Serial;
struct EspT{void restart(){}}; extern EspT ESP;
// FreeRTOS
typedef void* QueueHandle_t; typedef int BaseType_t;
#define pdTRUE 1
QueueHandle_t xQueueCreate(int,int); BaseType_t xQueueSend(QueueHandle_t,const void*,int); BaseType_t xQueueReceive(QueueHandle_t,void*,int); int uxQueueMessagesWaiting(QueueHandle_t);
void vTaskDelay(int); void xTaskCreatePinnedToCore(void(*)(void*),const char*,int,void*,int,void*,int);
struct portMUX_TYPE{int x;};
#define portMUX_INITIALIZER_UNLOCKED {0}
inline void portENTER_CRITICAL(portMUX_TYPE*){} inline void portEXIT_CRITICAL(portMUX_TYPE*){}
