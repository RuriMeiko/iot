/*
 * Sensors management for Smart Environment Monitoring System
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "globals.h"
#include <DHT.h>

// Function declarations
void initSensors();
float readTemperature();
float readHumidity();
bool checkMotion();
void updateMotionStatus();

extern DHT dht;

#endif // SENSORS_H