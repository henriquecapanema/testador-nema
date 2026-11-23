// samples.h — um timer grava posição, alvo e velocidade 100 vezes por segundo num
// buffer circular (~10 s); a página pede só as amostras novas.
#pragma once
#include <Arduino.h>

struct Sample { uint32_t t; float pos, tgt, rpm; };
constexpr uint32_t RING = 1024;
extern Sample ring[RING];
extern volatile uint32_t sampleSeq;
extern portMUX_TYPE ringMux;

void startSampling();
