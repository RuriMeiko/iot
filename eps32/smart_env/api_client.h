/*
 * API client for Smart Environment Monitoring System
 */

#ifndef API_CLIENT_H
#define API_CLIENT_H

#include "globals.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Function declarations
void sendDataToServer(float temperature, float humidity, bool motionDetected);

#endif // API_CLIENT_H