/*
 * File: common.h
 * Mô tả: Khai báo các thư viện và biến toàn cục dùng chung cho toàn bộ hệ thống
 */

#ifndef COMMON_H
#define COMMON_H

// Các thư viện Arduino cơ bản
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Khai báo extern các biến toàn cục sẽ được định nghĩa trong các file .cpp
extern LiquidCrystal_I2C lcd;

#endif // COMMON_H