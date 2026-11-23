#pragma once
#include "Arduino.h"
struct Preferences{bool begin(const char*,bool=false){return true;} void end(){} bool clear(){return true;}
 String getString(const char*,const String& ={}){return "";} size_t putString(const char*,const String&){return 0;}
 int32_t getInt(const char*,int32_t d=0){return d;} size_t putInt(const char*,int32_t){return 0;}
 float getFloat(const char*,float d=0){return d;} size_t putFloat(const char*,float){return 0;}
 bool getBool(const char*,bool d=false){return d;} size_t putBool(const char*,bool){return 0;}
 uint8_t getUChar(const char*,uint8_t d=0){return d;} size_t putUChar(const char*,uint8_t){return 0;}};
