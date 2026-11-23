// web.h — servidor HTTP (página, /status, /cmd, /scan, /wifi, /ota, /motor, /waypoints, /safety)
// e WebSocket na porta 81 (status empurrado a cada 100 ms + comandos).
#pragma once
#include <Arduino.h>

void webBegin();
