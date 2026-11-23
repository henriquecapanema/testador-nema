#pragma once
#include "Arduino.h"
enum HTTPMethod{HTTP_ANY,HTTP_GET,HTTP_POST};
struct WebServer{WebServer(int){} String arg(const char*){return "";} bool hasArg(const char*){return false;}
 void send(int,const char* =nullptr,const String& ={}){} void send_P(int,PGM_P,PGM_P){} void send_P(int,PGM_P,PGM_P,size_t){} void sendHeader(const char*,const char*){}
 String header(const char*){return "";} void collectHeaders(const char**,size_t){}
 void on(const char*,std::function<void()>){} void on(const char*,HTTPMethod,std::function<void()>){} void handleClient(){} void begin(){}};
