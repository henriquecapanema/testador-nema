// motion.cpp — o motor do nemabot. Um passo por vez, sem travar: a cada passo recalcula a
// velocidade em direção ao alvo, por isso o alvo pode mudar no meio do movimento.
#include "motion.h"
#include "log.h"
#include <Preferences.h>

// Micropasso, redução, rampa, batentes e ENA ficam salvos na memória do ESP32 (NVS,
// namespace "motor") e se mudam pela página. Os valores abaixo são só o padrão.
// Redução 1 = motor solto: os ângulos passam a ser do próprio eixo do motor.
int   MICROSTEP = 8;                              // tem que bater com as chaves do driver
float RATIO     = 20.0;                           // redução do J2 (1 = sem redução)
long  STEPS_REV = 200L * MICROSTEP;               // passos por volta do MOTOR
float STEPS_OUT_REV = STEPS_REV * RATIO;          // passos por volta da SAÍDA
float RPM_MAX = MOTOR_RPM_MAX / 20.0;             // limite na saída (recalculado com a redução)

Waypoint wpBuf[8];                                // escrito pela web (núcleo 0) sob wpMux
volatile uint8_t wpCount = 0;
volatile bool wpLoop = false;
portMUX_TYPE wpMux = portMUX_INITIALIZER_UNLOCKED;

QueueHandle_t cmdQueue;

// Estado do movimento: escrito só pelo loop() do motor (núcleo 1), lido pela página.
volatile long  pos = 0;           // posição atual, passos do motor
volatile long  target = 0;        // alvo, passos do motor
volatile float vel = 0;           // velocidade atual com sinal, passos/s
volatile float rpm = 5.0;         // velocidade escolhida, rpm na saída
volatile float acc = 4000.0;      // aceleração, passos/s² (no motor)
volatile uint8_t rampMode = 0;    // 0 = trapezoidal, 1 = curva S
volatile float jerk = 40000.0;    // passos/s³ (curva S)
float curAcc = 0;                 // aceleração instantânea na curva S
volatile Mode  mode = M_IDLE;
volatile long  cycles = 0;

// batentes virtuais e ENA (NVS)
volatile bool  limEn = false;
volatile float limMin = -120.0, limMax = 120.0;
volatile bool  enaUse = false, enaInv = true, enaOn = true;
bool pendEnaOff = false;

int lastDir = 0;
uint32_t nextStepUs = 0;
bool  pendCfg = false;             // config do motor esperando o motor parar
int   pendMs = 8;
float pendRatio = 20;
volatile uint32_t loopBeat = 0;   // millis() da última volta do loop() do motor (diagnóstico)

// Sequenciador das rotinas de teste
long seqBase = 0, seqAmp = 0;
volatile int  seqPhase = 0;
volatile bool dwelling = false;
uint32_t dwellUntil = 0;
volatile long seqCycles = 0;      // loop e repetibilidade (0 = sem fim no loop)
uint32_t seqDwell = 300;
volatile float sweepRpm = 0;      // varredura de rpm
float sweepStep = 0, sweepMax = 0;
volatile float sweepAcc = 0;      // varredura de aceleração
float accStep = 0, accMax = 0;
uint32_t resStart = 0;            // ressonância
float resDur = 30, resMin = 0, resMax = 0;
int   resDir = 1;
volatile float resRpmNow = 0;
uint32_t holdMs = 3000;           // folga
float stStep = 0.25;              // atrito estático
volatile int stCount = 10, stK = 0;
volatile int wpIdx = 0;           // trajetória
float wpRpmNow = 0;
volatile int jogDir = 1;          // jog
float jogScale = 1;
uint32_t jogLast = 0;

// ---------------- Configuração salva (NVS) ----------------
void applyMotorCfg() {
  STEPS_REV = 200L * MICROSTEP;
  STEPS_OUT_REV = STEPS_REV * RATIO;
  RPM_MAX = MOTOR_RPM_MAX / RATIO;
}

void loadMotorCfg() {
  Preferences p;
  p.begin(MOTOR_NS, true);
  MICROSTEP = p.getInt("ms", MICROSTEP);
  RATIO = p.getFloat("ratio", RATIO);
  limEn  = p.getBool("limEn", false);
  limMin = p.getFloat("limMin", -120.0);
  limMax = p.getFloat("limMax", 120.0);
  enaUse = p.getBool("enaUse", false);
  enaInv = p.getBool("enaInv", true);
  rampMode = p.getUChar("ramp", 0) ? 1 : 0;
  jerk = p.getFloat("jerk", 40000.0);
  p.end();
  if (MICROSTEP < 1 || MICROSTEP > 256) MICROSTEP = 8;
  if (RATIO < 0.1f || RATIO > 1000) RATIO = 20.0;
  if (!(limMin < limMax)) { limMin = -120.0; limMax = 120.0; limEn = false; }
  if (jerk < 1000 || jerk > 1e7f) jerk = 40000.0;
  applyMotorCfg();
  if (rpm > RPM_MAX) rpm = RPM_MAX;
}

bool validMotorCfg(int ms, float ratio) {
  bool pow2 = ms > 0 && (ms & (ms - 1)) == 0;
  return pow2 && ms <= 256 && ratio >= 0.1f && ratio <= 1000;
}

void saveMotorCfg(int ms, float ratio) {
  Preferences p;
  p.begin(MOTOR_NS, false);
  p.putInt("ms", ms);
  p.putFloat("ratio", ratio);
  p.end();
  logPrintf("Motor: micropasso 1/%d, redução %.2f:1 salvos.\n", ms, ratio);
}

void saveSafetyCfg(bool le, float lo, float hi, bool eu, bool ei) {
  Preferences p;
  p.begin(MOTOR_NS, false);
  p.putBool("limEn", le);
  p.putFloat("limMin", lo);
  p.putFloat("limMax", hi);
  p.putBool("enaUse", eu);
  p.putBool("enaInv", ei);
  p.end();
  logPrintf("Segurança: batentes %s (%.1f° a %.1f°), ENA %s%s.\n", le ? "ligados" : "desligados", lo, hi,
            eu ? "ligado" : "desligado", eu ? (ei ? ", ativo em LOW" : ", ativo em HIGH") : "");
}

void saveRampCfg(uint8_t r, float j) {
  Preferences p;
  p.begin(MOTOR_NS, false);
  p.putUChar("ramp", r);
  p.putFloat("jerk", j);
  p.end();
}

// ---------------- Helpers do movimento ----------------
long  degToSteps(float deg)  { return lround(deg / 360.0 * STEPS_OUT_REV); }
float stepsToDeg(long steps) { return steps * 360.0 / STEPS_OUT_REV; }

// Batentes virtuais: nenhum alvo passa de [limMin, limMax] quando ligados.
long clampSteps(long s) {
  if (!limEn) return s;
  long lo = degToSteps(limMin), hi = degToSteps(limMax);
  return s < lo ? lo : (s > hi ? hi : s);
}
void setT(long s) { target = clampSteps(s); }

float currentVmax() {
  float r = rpm;
  if (mode == M_SWEEP) r = sweepRpm;
  else if (mode == M_WAYPOINTS && wpRpmNow > 0) r = wpRpmNow;
  else if (mode == M_RESONANCE) r = resRpmNow;
  else if (mode == M_JOG) r = rpm * jogScale;
  if (r > RPM_MAX) r = RPM_MAX;
  return r * STEPS_OUT_REV / 60.0;
}

float accNow() { return mode == M_ACC_SWEEP ? sweepAcc : acc; }

// Quantos passos ainda andaria freando a partir da velocidade atual.
long stopDistance() {
  float sv = fabsf(vel);
  float v0 = min(V_START, currentVmax());
  float s = (sv * sv - v0 * v0) / (2 * accNow());
  return s > 0 ? (long)ceilf(s) : 0;
}

void stopRamp() {
  mode = M_IDLE; dwelling = false;
  long d = stopDistance();
  target = pos + (vel > 0 ? d : (vel < 0 ? -d : 0));
}

// Torque pelo pino ENA (TB6600: ENA+; TMC2209: EN). enaInv = ativo em LOW.
void setEna(bool on) {
  if (!enaUse) return;
  digitalWrite(PIN_ENA, (on ^ enaInv) ? HIGH : LOW);
  enaOn = on;
}
void ensureEna() {                 // qualquer movimento religa o torque
  pendEnaOff = false;
  if (enaUse && !enaOn) { setEna(true); delayMicroseconds(50); logPrintln("Torque religado."); }
}

void startSequence(Mode m, float ampDeg) {
  mode = m;
  seqBase = target;
  seqAmp = degToSteps(ampDeg);
  seqPhase = 0;
  dwelling = false;
  cycles = 0;
  setT(seqBase + seqAmp);
}

void finish(const char* msg) {
  mode = M_IDLE; dwelling = false;
  if (msg) logPrintln(msg);
}

static inline float orDef(float v, float d) { return v > 0 ? v : d; }

void applyCmd(const Cmd& c) {
  switch (c.type) {
    case C_GOTO:
      ensureEna(); mode = M_IDLE; dwelling = false;
      setT(degToSteps(constrain(c.p1, -3600.0f, 3600.0f)));
      break;
    case C_MOVE:
      ensureEna(); mode = M_IDLE; dwelling = false;
      setT(target + degToSteps(c.p1));
      break;
    case C_RPM:
      if (c.p1 > 0) rpm = min(c.p1, RPM_MAX);
      break;
    case C_ACC:
      if (c.p1 >= 100) acc = min(c.p1, ACC_MAX);
      break;
    case C_RAMP:
      rampMode = c.p1 > 0 ? 1 : 0;
      if (c.p2 > 0) jerk = constrain(c.p2, 1000.0f, 1e7f);
      break;
    case C_LOOP: {
      ensureEna();
      float amp = c.p1 != 0 ? c.p1 : 90;
      seqCycles = c.p2 > 0 ? lroundf(c.p2) : 0;
      seqDwell = (uint32_t)orDef(c.p3, 300);
      startSequence(M_LOOP, amp);
      if (seqCycles) logPrintf("Loop de %.1f° · %ld ciclos · pausa %lu ms\n", amp, (long)seqCycles, (unsigned long)seqDwell);
      else logPrintf("Loop de %.1f° sem fim · pausa %lu ms\n", amp, (unsigned long)seqDwell);
      break;
    }
    case C_SWEEP: {
      ensureEna();
      float lo = c.p2 > 0 ? c.p2 : RPM_MAX / 15, hi = c.p3 > 0 ? min(c.p3, RPM_MAX) : RPM_MAX;
      if (lo > hi) { float t = lo; lo = hi; hi = t; }
      int n = c.p4 >= 2 ? lroundf(c.p4) : 15;
      sweepRpm = lo; sweepMax = hi; sweepStep = (hi - lo) / (n - 1);
      startSequence(M_SWEEP, c.p1 != 0 ? c.p1 : 90);
      logPrintf("Varredura de rpm: %.1f → %.1f em %d degraus\n", lo, hi, n);
      break;
    }
    case C_ACC_SWEEP: {
      ensureEna();
      float lo = orDef(c.p2, 1000), hi = c.p3 > 0 ? min(c.p3, ACC_MAX) : 20000;
      if (lo > hi) { float t = lo; lo = hi; hi = t; }
      int n = c.p4 >= 2 ? lroundf(c.p4) : 10;
      sweepAcc = lo; accMax = hi; accStep = (hi - lo) / (n - 1);
      startSequence(M_ACC_SWEEP, c.p1 != 0 ? c.p1 : 90);
      logPrintf("Varredura de aceleração: %.0f → %.0f p/s² em %d degraus\n", lo, hi, n);
      break;
    }
    case C_RESONANCE:
      if (limEn) { logPrintln("Ressonância recusada: batentes ligados."); break; }
      ensureEna();
      resMin = c.p1 > 0 ? min(c.p1, RPM_MAX) : RPM_MAX / 15;
      resMax = c.p2 > 0 ? min(c.p2, RPM_MAX) : RPM_MAX;
      resDur = orDef(c.p3, 30);
      resDir = c.p4 < 0 ? -1 : 1;
      resStart = millis(); resRpmNow = resMin;
      mode = M_RESONANCE; dwelling = false;
      target = pos + resDir * BIG;
      logPrintf("Ressonância: %.1f → %.1f rpm em %.0f s\n", resMin, resMax, resDur);
      break;
    case C_BACKLASH: {
      ensureEna();
      float amp = c.p1 != 0 ? c.p1 : 10;
      holdMs = (uint32_t)orDef(c.p2, 3000);
      startSequence(M_BACKLASH, amp);
      logPrintf("Folga: ±%.2f°, segura %lu ms em 0°\n", amp, (unsigned long)holdMs);
      break;
    }
    case C_DRIFT: {
      ensureEna();
      float amp = c.p1 != 0 ? c.p1 : 90;
      seqCycles = c.p2 > 0 ? lroundf(c.p2) : 20;
      seqDwell = (uint32_t)orDef(c.p3, 100);
      startSequence(M_DRIFT, amp);
      logPrintf("Repetibilidade: %ld ciclos de %.1f°\n", (long)seqCycles, amp);
      break;
    }
    case C_STICTION:
      ensureEna();
      stStep = c.p1 != 0 ? c.p1 : 0.25;
      stCount = c.p2 > 0 ? lroundf(c.p2) : 10;
      seqDwell = (uint32_t)orDef(c.p3, 1000);
      mode = M_STICTION; seqBase = target; seqPhase = 0; stK = 1; dwelling = false; cycles = 0;
      setT(seqBase + degToSteps(stStep));
      logPrintf("Atrito estático: %d degraus de %.3f°\n", (int)stCount, stStep);
      break;
    case C_WAYPOINTS: {
      if (wpCount == 0) break;
      ensureEna();
      Waypoint w;
      portENTER_CRITICAL(&wpMux); w = wpBuf[0]; portEXIT_CRITICAL(&wpMux);
      mode = M_WAYPOINTS; wpIdx = 0; cycles = 0; dwelling = false;
      wpRpmNow = w.rpm; setT(degToSteps(w.deg));
      logPrintf("Trajetória: %d pontos%s\n", (int)wpCount, wpLoop ? " em loop" : "");
      break;
    }
    case C_JOG:
      ensureEna();
      jogDir = c.p1 < 0 ? -1 : 1;
      jogScale = c.p2 > 0 ? constrain(c.p2, 0.05f, 1.0f) : 1.0f;
      jogLast = millis();
      mode = M_JOG; dwelling = false;
      setT(pos + jogDir * BIG);
      break;
    case C_STOP:
      if (mode != M_IDLE && mode != M_JOG) logPrintf("Parado (ciclo %ld).\n", (long)cycles);
      stopRamp();
      break;
    case C_ZERO: {
      long p = pos;
      pos = pos - p;
      target = target - p;
      seqBase -= p;
      logPrintln("Posição zerada.");
      break;
    }
    case C_CFG:                         // nova config do motor: para e aplica quando parar
      pendMs = (int)c.p1; pendRatio = c.p2; pendCfg = true;
      stopRamp();
      break;
    case C_ENA:
      if (!enaUse) break;
      if (c.p1 > 0) { setEna(true); pendEnaOff = false; logPrintln("Torque ligado."); }
      else { stopRamp(); pendEnaOff = true; }       // solta só depois de parar
      break;
    case C_LIMITS: {
      limEn = c.p1 > 0; limMin = c.p2; limMax = c.p3;
      int f = (int)c.p4;
      bool use = f & 1;
      enaInv = (f & 2) != 0;
      if (use && !enaUse) { enaUse = true; pinMode(PIN_ENA, OUTPUT); setEna(true); }
      else if (use) setEna(enaOn);                 // reaplica com a lógica nova
      else if (enaUse) { setEna(true); enaUse = false; enaOn = true; }
      if (limEn && mode == M_RESONANCE) stopRamp();
      break;
    }
  }
}

// Pausa ao chegar em cada ponto, conforme a rotina e a fase.
uint32_t curDwell() {
  switch (mode) {
    case M_LOOP: case M_DRIFT: return seqDwell;
    case M_SWEEP: case M_ACC_SWEEP: return seqPhase == 0 ? 300 : 700;
    case M_BACKLASH: return (seqPhase == 1 || seqPhase == 3) ? holdMs : 300;
    case M_STICTION: return seqPhase == 0 ? seqDwell : 0;
    case M_WAYPOINTS: {
      uint32_t d;
      portENTER_CRITICAL(&wpMux); d = wpBuf[wpIdx < 8 ? wpIdx : 0].dwellMs; portEXIT_CRITICAL(&wpMux);
      return d;
    }
    default: return 0;
  }
}

// Próximo alvo de cada rotina, depois da pausa.
void seqStep() {
  switch (mode) {
    case M_LOOP:
      if (seqPhase == 0) { seqPhase = 1; setT(seqBase); return; }
      cycles++;
      if (seqCycles > 0 && cycles >= seqCycles) { logPrintf("Loop: %ld ciclos concluídos.\n", (long)cycles); finish(nullptr); return; }
      logPrintf("  ciclo %ld\n", (long)cycles);
      seqPhase = 0; setT(seqBase + seqAmp); return;
    case M_SWEEP:
      if (seqPhase == 0) { seqPhase = 1; setT(seqBase); return; }
      cycles++; sweepRpm = sweepRpm + sweepStep;
      if (sweepStep <= 0 || sweepRpm > sweepMax * 1.0001f) { finish("Varredura de rpm encerrada. Anote o rpm em que o motor perdeu passo."); return; }
      logPrintf("  %.1f rpm na saída (%.0f no motor)\n", (float)sweepRpm, sweepRpm * RATIO);
      seqPhase = 0; setT(seqBase + seqAmp); return;
    case M_ACC_SWEEP:
      if (seqPhase == 0) { seqPhase = 1; setT(seqBase); return; }
      cycles++; sweepAcc = sweepAcc + accStep;
      if (accStep <= 0 || sweepAcc > accMax * 1.0001f) { finish("Varredura de aceleração encerrada."); return; }
      logPrintf("  %.0f p/s²\n", (float)sweepAcc);
      seqPhase = 0; setT(seqBase + seqAmp); return;
    case M_BACKLASH:
      if (seqPhase == 0) { seqPhase = 1; setT(seqBase); return; }
      if (seqPhase == 1) { seqPhase = 2; setT(seqBase - seqAmp); return; }
      if (seqPhase == 2) { seqPhase = 3; setT(seqBase); return; }
      finish("Folga: medição concluída (segurou em 0° vindo de + e de −)."); return;
    case M_DRIFT:
      if (seqPhase == 0) { seqPhase = 1; setT(seqBase); return; }
      cycles++;
      if (cycles >= seqCycles) { logPrintf("Repetibilidade: %ld ciclos, parado em 0°. Confira a marca física.\n", (long)cycles); finish(nullptr); return; }
      seqPhase = 0; setT(seqBase + seqAmp); return;
    case M_STICTION:
      if (seqPhase == 1) { finish("Atrito estático: voltou à posição inicial."); return; }
      if (stK < stCount) { stK = stK + 1; setT(seqBase + degToSteps(stStep * stK)); return; }
      seqPhase = 1; setT(seqBase); return;
    case M_WAYPOINTS: {
      int i = wpIdx + 1;
      if (i >= wpCount) {
        if (!wpLoop) { finish("Trajetória concluída."); return; }
        i = 0; cycles++;
      }
      wpIdx = i;
      Waypoint w;
      portENTER_CRITICAL(&wpMux); w = wpBuf[i]; portEXIT_CRITICAL(&wpMux);
      wpRpmNow = w.rpm; setT(degToSteps(w.deg));
      return;
    }
    default: return;
  }
}

// Chamado quando o eixo parou no alvo: pausa e depois segue a rotina.
void onArrived() {
  if (mode == M_IDLE || mode == M_JOG || mode == M_RESONANCE) return;
  if (!dwelling) { dwelling = true; dwellUntil = millis() + curDwell(); return; }
  if ((int32_t)(millis() - dwellUntil) < 0) return;
  dwelling = false;
  seqStep();
}

// Decide a velocidade do próximo passo: acelera, mantém ou freia em direção ao alvo.
void updateVelocity() {
  long d = target - pos;
  int sd = (d > 0) - (d < 0);                 // sentido para o alvo
  int sv_sign = (vel > 0) - (vel < 0);        // sentido atual
  float sv = fabsf(vel);
  float vmax = currentVmax();
  float v0 = min(V_START, vmax);
  float a = accNow();

  if (sv_sign == 0) {                         // parado: parte em v0
    vel = sd * v0;
    curAcc = a * 0.1f;
    return;
  }

  if (rampMode == 0) {                        // trapezoidal: v² muda 2·a a cada passo
    float stopSteps = (sv * sv - v0 * v0) / (2 * a);
    bool brake = (sd != sv_sign) || (labs(d) <= stopSteps) || (sv > vmax * 1.001f);
    if (brake) {
      float s2 = sv * sv - 2 * a;
      if (s2 <= v0 * v0) vel = (sd == sv_sign) ? sv_sign * v0 : sd * v0;
      else vel = sv_sign * sqrtf(s2);
    } else if (sv < vmax) {
      vel = sv_sign * min(sqrtf(sv * sv + 2 * a), vmax);
    }
    return;
  }

  // Curva S: a aceleração muda no máximo 'jerk' por segundo (Δa = jerk·dt, dt = 1/v).
  float jstep = jerk / max(sv, v0), aMin = a * 0.1f;
  float aReq = (sd == sv_sign && d != 0) ? (sv * sv - v0 * v0) / (2.0f * labs(d)) : 1e30f;  // para exatamente no alvo
  bool over = sv > vmax * 1.001f;
  bool brake = (sd != sv_sign) || over || aReq >= a * 0.5f;
  if (brake) {
    float dec;
    if (sd != sv_sign || over) { curAcc = min(a, curAcc + jstep); dec = curAcc; }
    else {
      float want = min(a, aReq);
      curAcc = curAcc < want ? min(want, curAcc + jstep) : max(want, curAcc - jstep);
      dec = aReq > a * 0.9f ? min(aReq, a * 1.5f) : curAcc;
    }
    float s2 = sv * sv - 2 * dec;
    if (s2 <= v0 * v0) vel = (sd == sv_sign) ? sv_sign * v0 : sd * v0;
    else vel = sv_sign * sqrtf(s2);
  } else if (sv < vmax) {
    bool taper = (vmax - sv) <= (curAcc + aMin) * 0.5f * (curAcc - aMin) / jerk;   // perto do cruzeiro: suaviza
    float want = taper ? aMin : a;
    curAcc = curAcc < want ? min(want, curAcc + jstep) : max(want, curAcc - jstep);
    vel = sv_sign * min(sqrtf(sv * sv + 2 * curAcc), vmax);
  } else {
    curAcc = max(aMin, curAcc - jstep);
  }
}

void motionService() {
  Cmd c;
  while (xQueueReceive(cmdQueue, &c, 0) == pdTRUE) applyCmd(c);

  if (mode == M_JOG && millis() - jogLast > 450) {        // jog sem renovação: Wi-Fi caiu ou botão solto
    logPrintln("Jog: sem renovação há 450 ms, parando.");
    stopRamp();
  }
  if (mode == M_RESONANCE) {
    float el = (millis() - resStart) / 1000.0f;
    if (el >= resDur) { logPrintln("Ressonância concluída."); stopRamp(); }
    else { resRpmNow = resMin + (resMax - resMin) * el / resDur; target = pos + resDir * BIG; }
  }

  if (vel == 0 && pos == target) {
    if (pendCfg) {                     // parou: aplica micropasso e redução novos
      pendCfg = false;
      MICROSTEP = pendMs; RATIO = pendRatio;
      applyMotorCfg();
      if (rpm > RPM_MAX) rpm = RPM_MAX;
      pos = 0; target = 0; seqBase = 0;
      logPrintf("Motor: micropasso 1/%d, redução %.2f:1 aplicados. Posição zerada.\n", MICROSTEP, RATIO);
    }
    if (pendEnaOff) { pendEnaOff = false; setEna(false); logPrintln("Torque solto."); }
    onArrived();
    return;
  }

  uint32_t now = micros();
  if (vel != 0 && (int32_t)(now - nextStepUs) < 0) return;   // ainda não é hora do passo

  bool wasIdle = (vel == 0);
  updateVelocity();
  if (vel == 0) return;

  int s = vel > 0 ? 1 : -1;
  if (s != lastDir) {
    digitalWrite(PIN_DIR, s > 0 ? HIGH : LOW);
    lastDir = s;
    delayMicroseconds(20);
  }
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(5);
  digitalWrite(PIN_STEP, LOW);
  pos = pos + s;

  uint32_t period = (uint32_t)(1e6f / fabsf(vel));
  if (wasIdle || (int32_t)(now - nextStepUs) > (int32_t)period) nextStepUs = now + period;
  else nextStepUs += period;
}

// Alvo mostrado na página: no jog e na ressonância o alvo interno é "infinito".
long shownTarget() { return (mode == M_JOG || mode == M_RESONANCE) ? pos : target; }

// Fase atual da rotina, curta, para a página.
void subModeStr(char* b, size_t n) {
  switch (mode) {
    case M_LOOP: if (seqCycles) snprintf(b, n, "ciclo %ld/%ld", (long)cycles + 1, (long)seqCycles); else snprintf(b, n, "ciclo %ld", (long)cycles + 1); break;
    case M_SWEEP: snprintf(b, n, "%.1f rpm", (float)sweepRpm); break;
    case M_ACC_SWEEP: snprintf(b, n, "%.0f p/s²", (float)sweepAcc); break;
    case M_RESONANCE: snprintf(b, n, "%.1f rpm", (float)resRpmNow); break;
    case M_BACKLASH: {
      const char* ph[] = { "→ +", "→ 0", "→ −", "→ 0" };
      int p = seqPhase & 3;
      snprintf(b, n, "%s", dwelling && p == 1 ? "hold+" : dwelling && p == 3 ? "hold-" : ph[p]);
      break;
    }
    case M_DRIFT: snprintf(b, n, "%ld/%ld", (long)cycles + 1, (long)seqCycles); break;
    case M_STICTION: if (seqPhase) snprintf(b, n, "voltando"); else snprintf(b, n, "%d/%d", (int)stK, (int)stCount); break;
    case M_WAYPOINTS: snprintf(b, n, "wp %d/%d", (int)wpIdx + 1, (int)wpCount); break;
    case M_JOG: snprintf(b, n, "jog %s", jogDir > 0 ? "+" : "−"); break;
    default: b[0] = 0;
  }
}

const char* modeName(Mode m) {
  switch (m) {
    case M_LOOP: return "loop"; case M_SWEEP: return "sweep"; case M_ACC_SWEEP: return "accsweep";
    case M_RESONANCE: return "resonance"; case M_BACKLASH: return "backlash"; case M_DRIFT: return "drift";
    case M_STICTION: return "stiction"; case M_WAYPOINTS: return "waypoints"; case M_JOG: return "jog";
    default: return "idle";
  }
}

bool sendCmdN(CmdType t, float p1, float p2, float p3, float p4) {
  Cmd c = { t, p1, p2, p3, p4 };
  return xQueueSend(cmdQueue, &c, 0) == pdTRUE;
}
bool sendCmd2(CmdType t, float v, float v2) { return sendCmdN(t, v, v2, 0, 0); }
bool sendCmd(CmdType t, float v) { return sendCmdN(t, v, 0, 0, 0); }
