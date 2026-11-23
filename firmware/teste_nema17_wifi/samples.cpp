// samples.cpp — amostras do gráfico a 100 Hz.
#include "samples.h"
#include "motion.h"
#include "esp_timer.h"

const uint32_t SAMPLE_US = 10000;          // 10 ms = 100 Hz
Sample ring[RING];
volatile uint32_t sampleSeq = 0;           // total de amostras já gravadas
portMUX_TYPE ringMux = portMUX_INITIALIZER_UNLOCKED;


void sampleCb(void*) {
  Sample s;
  s.t   = (uint32_t)(esp_timer_get_time() / 1000);
  s.pos = stepsToDeg(pos);
  s.tgt = stepsToDeg(shownTarget());
  s.rpm = vel * 60.0f / STEPS_OUT_REV;
  portENTER_CRITICAL(&ringMux);
  ring[sampleSeq % RING] = s;
  sampleSeq = sampleSeq + 1;
  portEXIT_CRITICAL(&ringMux);
}

void startSampling() {
  esp_timer_create_args_t args = {};
  args.callback = &sampleCb;
  args.name = "amostras";
  esp_timer_handle_t h;
  esp_timer_create(&args, &h);
  esp_timer_start_periodic(h, SAMPLE_US);
}
