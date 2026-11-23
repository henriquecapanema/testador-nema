// motion.h — motor de passo: rampas (trapezoidal/curva S), rotinas de teste, batentes,
// ENA e a fila de comandos. Roda no loop() (núcleo 1); a web só manda comandos pela fila.
#pragma once
#include <Arduino.h>
#include "config.h"

// ---------------- Comandos ----------------
enum Mode : uint8_t {
  M_IDLE, M_LOOP, M_SWEEP, M_ACC_SWEEP, M_RESONANCE,
  M_BACKLASH, M_DRIFT, M_STICTION, M_WAYPOINTS, M_JOG
};
enum CmdType : uint8_t {
  C_GOTO, C_MOVE, C_RPM, C_ACC, C_RAMP, C_LOOP, C_SWEEP,
  C_ACC_SWEEP, C_RESONANCE, C_BACKLASH, C_DRIFT, C_STICTION,
  C_WAYPOINTS, C_JOG, C_STOP, C_ZERO, C_CFG, C_ENA, C_LIMITS
};
struct Cmd { CmdType type; float p1; float p2; float p3; float p4; };

struct Waypoint { float deg; float rpm; uint32_t dwellMs; };

// configuração (NVS)
extern int   MICROSTEP;
extern float RATIO;
extern float STEPS_OUT_REV;
extern float RPM_MAX;

// estado lido pela página e pelo Serial
extern QueueHandle_t cmdQueue;
extern volatile long  pos, target, cycles;
extern volatile float vel, rpm, acc, jerk, sweepRpm, sweepAcc, resRpmNow;
extern volatile uint8_t rampMode;
extern volatile Mode  mode;
extern volatile bool  limEn, enaUse, enaInv, enaOn;
extern volatile float limMin, limMax;
extern volatile uint32_t loopBeat;
extern Waypoint wpBuf[8];
extern volatile uint8_t wpCount;
extern volatile bool wpLoop;
extern portMUX_TYPE wpMux;

void loadMotorCfg();
bool validMotorCfg(int ms, float ratio);
void saveMotorCfg(int ms, float ratio);
void saveSafetyCfg(bool le, float lo, float hi, bool eu, bool ei);
void saveRampCfg(uint8_t r, float j);
long  degToSteps(float deg);
float stepsToDeg(long steps);
void setEna(bool on);
void motionService();
long shownTarget();
void subModeStr(char* b, size_t n);
const char* modeName(Mode m);
bool sendCmdN(CmdType t, float p1, float p2, float p3, float p4);
bool sendCmd2(CmdType t, float v, float v2);
bool sendCmd(CmdType t, float v);
