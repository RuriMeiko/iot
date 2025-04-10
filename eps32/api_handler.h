/*
 * File: api_handler.h
 * Mu00f4 tu1ea3: Quu1ea3n lu00fd giao tiu1ebfp API - gu1eedi vu00e0 nhu1eadn du1eef liu1ec7u tu1eeb server
 */

#ifndef API_HANDLER_H
#define API_HANDLER_H

#include "common.h"

// ======= KHAI Bu00c1O Cu00c1C Hu00c0M =======

/**
 * Khu1edfi tu1ea1o API handler
 */
void setupAPI();

/**
 * Xu1eed lu00fd giao tiu1ebfp API - gu1eedi vu00e0 nhu1eadn du1eef liu1ec7u
 * Cu1ea7n gu1ecdi hu00e0m nu00e0y trong vu00f2ng lu1eb7p chu00ednh nu1ebfu u0111u00e3 ku1ebft nu1ed1i WiFi
 */
void handleAPI();

/**
 * Gu1eedi du1eef liu1ec7u lu00ean server
 * @param temperature Nhiu1ec7t u0111u1ed9 tu1eeb cu1ea3m biu1ebfn
 * @param humidity    u0110u1ed9 u1ea9m tu1eeb cu1ea3m biu1ebfn
 * @param motion      Tru1ea1ng thu00e1i cu1ea3m biu1ebfn chuyu1ec3n u0111u1ed9ng
 * @return            true nu1ebfu gu1eedi thu00e0nh cu00f4ng, false nu1ebfu thu1ea5t bu1ea1i
 */
bool sendDataToAPI(float temperature, float humidity, bool motion);

/**
 * Xu1eed lu00fd phu1ea3n hu1ed3i nhu1eadn u0111u01b0u1ee3c tu1eeb API
 * @param response Chuyu1ed7i JSON nhu1eadn u0111u01b0u1ee3c tu1eeb API
 */
void processAPIResponse(const String& response);

#endif // API_HANDLER_H