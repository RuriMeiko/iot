/*
 * Display handling for Smart Environment Monitoring System
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "globals.h"
#include "characters.h"

// Function declarations
void initDisplay();
void displayWelcomeScreen();
void displayLoadingAnimation();
void updateLCD(LcdState state);
void displaySensorData(bool isWiFiConnected, bool motionDetected, bool isApiConnected, float temperature, float humidity);

#endif // DISPLAY_H