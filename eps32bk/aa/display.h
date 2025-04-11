#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

// Display function prototypes
void initDisplay();
void displayWelcomeScreen();
void displayLoadingAnimation();
void updateLCD();
void displaySensorData();
void displayDeviceStatus();

// External reference to the global LCD object
extern LiquidCrystal_I2C LCD;

#endif // DISPLAY_H