#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <DHT.h>

// Function prototypes for sensors
void setupSensors();
void checkMotion();

// External reference to the DHT sensor
extern DHT dht;

#endif // SENSORS_H