#ifndef DEVICE_CONTROL_H
#define DEVICE_CONTROL_H

#include <Arduino.h>
#include <ArduinoJson.h>

// Function prototypes for device control
void setupDeviceControl();
void handleDeviceControl(const JsonDocument& doc);

#endif // DEVICE_CONTROL_H