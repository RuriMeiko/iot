/*
 * File: common.h
 * Description: Declaration of libraries and global variables used throughout the system
 */

#ifndef COMMON_H
#define COMMON_H

// Basic Arduino libraries
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Declare extern global variables that will be defined in .cpp files
extern LiquidCrystal_I2C lcd;

#endif // COMMON_H