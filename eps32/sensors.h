/*
 * File: sensors.h
 * Description: Declaration of functions and variables for sensor management
 * Includes: DHT11 (temperature, humidity) and motion sensor
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "common.h"

// Define sensor connection pins
#define DHTPIN 4       // DHT11 sensor pin
#define DHTTYPE DHT11  // DHT sensor type
#define MOTION_PIN 5   // Motion sensor pin
#define LED_PIN 2      // LED pin

// Global variable declaration
extern DHT dht;

// ======= FUNCTION DECLARATIONS =======

/**
 * Initialize sensors and GPIO pins
 */
void setupSensors();

/**
 * Process data from sensors
 * - Read DHT11 periodically
 * - Check motion sensor status
 * - Control LED when motion is detected
 */
void handleSensors();

/**
 * Get current temperature value
 * @return Temperature measured from DHT11 (u00b0C)
 */
float getTemperature();

/**
 * Get current humidity value
 * @return Humidity measured from DHT11 (%)
 */
float getHumidity();

/**
 * Check motion sensor status
 * @return true if motion is detected, false if not
 */
bool isMotionDetected();

#endif // SENSORS_H