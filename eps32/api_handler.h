/*
 * File: api_handler.h
 * Description: Manage API interface - sending and receiving data from server
 */

#ifndef API_HANDLER_H
#define API_HANDLER_H

#include "common.h"

// ======= FUNCTION DECLARATIONS =======

/**
 * Initialize API handler
 */
void setupAPI();

/**
 * Handle API interface - sending and receiving data
 * This function should be called in the main loop if WiFi is connected
 */
void handleAPI();

/**
 * Send data to server
 * @param temperature Temperature from sensor
 * @param humidity    Humidity from sensor
 * @param motion      Motion sensor state
 * @return            true if sent successfully, false if failed
 */
bool sendDataToAPI(float temperature, float humidity, bool motion);

/**
 * Process response received from API
 * @param response JSON string received from API
 */
void processAPIResponse(const String& response);

#endif // API_HANDLER_H