/*
 * File: sensors.h
 * Mô tả: Khai báo các hàm và biến cho việc quản lý cảm biến
 * Bao gồm: DHT11 (nhiệt độ, độ ẩm) và cảm biến chuyển động
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <DHT.h>

// Định nghĩa chân kết nối cảm biến
#define DHTPIN 4       // Chân kết nối cảm biến DHT11
#define DHTTYPE DHT11  // Loại cảm biến DHT
#define MOTION_PIN 5   // Chân kết nối cảm biến chuyển động
#define LED_PIN 2      // Chân kết nối LED

// ======= KHAI BÁO CÁC HÀM =======

/**
 * Khởi tạo các cảm biến và chân GPIO
 */
void setupSensors();

/**
 * Xử lý dữ liệu từ các cảm biến
 * - Đọc DHT11 định kỳ
 * - Kiểm tra trạng thái cảm biến chuyển động
 * - Điều khiển đèn LED khi có chuyển động
 */
void handleSensors();

/**
 * Lấy giá trị nhiệt độ hiện tại
 * @return Nhiệt độ đo được từ DHT11 (°C)
 */
float getTemperature();

/**
 * Lấy giá trị độ ẩm hiện tại
 * @return Độ ẩm đo được từ DHT11 (%)
 */
float getHumidity();

/**
 * Kiểm tra trạng thái cảm biến chuyển động
 * @return true nếu phát hiện chuyển động, false nếu không
 */
bool isMotionDetected();

#endif // SENSORS_H