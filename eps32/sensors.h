/*
 * File: sensors.h
 * Mu00f4 tu1ea3: Khai bu00e1o cu00e1c hu00e0m vu00e0 biu1ebfn cho viu1ec7c quu1ea3n lu00fd cu1ea3m biu1ebfn
 * Bao gu1ed3m: DHT11 (nhiu1ec7t u0111u1ed9, u0111u1ed9 u1ea9m) vu00e0 cu1ea3m biu1ebfn chuyu1ec3n u0111u1ed9ng
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "common.h"

// u0110u1ecbnh nghu0129a chu00e2n ku1ebft nu1ed1i cu1ea3m biu1ebfn
#define DHTPIN 4       // Chu00e2n ku1ebft nu1ed1i cu1ea3m biu1ebfn DHT11
#define DHTTYPE DHT11  // Lou1ea1i cu1ea3m biu1ebfn DHT
#define MOTION_PIN 5   // Chu00e2n ku1ebft nu1ed1i cu1ea3m biu1ebfn chuyu1ec3n u0111u1ed9ng
#define LED_PIN 2      // Chu00e2n ku1ebft nu1ed1i LED

// Khai bu00e1o biu1ebfn tou00e0n cu1ee5c
extern DHT dht;

// ======= KHAI Bu00c1O Cu00c1C Hu00c0M =======

/**
 * Khu1edfi tu1ea1o cu00e1c cu1ea3m biu1ebfn vu00e0 chu00e2n GPIO
 */
void setupSensors();

/**
 * Xu1eed lu00fd du1eef liu1ec7u tu1eeb cu00e1c cu1ea3m biu1ebfn
 * - u0110u1ecdc DHT11 u0111u1ecbnh ku1ef3
 * - Kiu1ec3m tra tru1ea1ng thu00e1i cu1ea3m biu1ebfn chuyu1ec3n u0111u1ed9ng
 * - u0110iu1ec1u khiu1ec3n u0111u00e8n LED khi cu00f3 chuyu1ec3n u0111u1ed9ng
 */
void handleSensors();

/**
 * Lu1ea5y giu00e1 tru1ecb nhiu1ec7t u0111u1ed9 hiu1ec7n tu1ea1i
 * @return Nhiu1ec7t u0111u1ed9 u0111o u0111u01b0u1ee3c tu1eeb DHT11 (u00b0C)
 */
float getTemperature();

/**
 * Lu1ea5y giu00e1 tru1ecb u0111u1ed9 u1ea9m hiu1ec7n tu1ea1i
 * @return u0110u1ed9 u1ea9m u0111o u0111u01b0u1ee3c tu1eeb DHT11 (%)
 */
float getHumidity();

/**
 * Kiu1ec3m tra tru1ea1ng thu00e1i cu1ea3m biu1ebfn chuyu1ec3n u0111u1ed9ng
 * @return true nu1ebfu phu00e1t hiu1ec7n chuyu1ec3n u0111u1ed9ng, false nu1ebfu khu00f4ng
 */
bool isMotionDetected();

#endif // SENSORS_H