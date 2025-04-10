/*
 * File: lcd_display.h
 * Mô tả: Quản lý màn hình LCD 16x2, bao gồm cấu hình và hiển thị
 */

#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// Định nghĩa các custom character cho LCD
#define WIFI_ICON   0  // Biểu tượng WiFi
#define TEMP_ICON   1  // Biểu tượng nhiệt độ
#define HUMID_ICON  2  // Biểu tượng độ ẩm
#define MOTION_ICON 3  // Biểu tượng chuyển động

// ======= KHAI BÁO CÁC HÀM =======

/**
 * Khởi tạo màn hình LCD và tạo custom characters
 */
void setupLCD();

/**
 * Hiển thị màn hình khởi động
 */
void displayBootScreen();

/**
 * Hiển thị thông tin về quá trình kết nối WiFi
 * @param message Thông báo cần hiển thị
 * @param ssid    Tên mạng WiFi đang kết nối (tuỳ chọn)
 */
void displayWiFiConnecting(const String& message, const String& ssid = "");

/**
 * Hiển thị thông tin về chế độ AP
 * @param apName Tên của Access Point
 */
void displayAPMode(const String& apName);

/**
 * Cập nhật thông tin hiển thị trên LCD
 * @param temperature Nhiệt độ (°C)
 * @param humidity    Độ ẩm (%)
 * @param motion      Trạng thái cảm biến chuyển động
 * @param wifiStatus  Trạng thái kết nối WiFi
 */
void updateDisplay(float temperature, float humidity, bool motion, bool wifiStatus);

/**
 * Hiển thị thông báo lỗi
 * @param error Thông báo lỗi cần hiển thị
 */
void displayError(const String& error);

/**
 * Truy cập đối tượng LCD để các module khác có thể sử dụng
 * @return Tham chiếu đến đối tượng LCD
 */
LiquidCrystal_I2C& getLCD();

#endif // LCD_DISPLAY_H