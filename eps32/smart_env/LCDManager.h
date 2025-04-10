#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// Enum for different screen states
enum LcdState {
    STARTING,
    CONNECTING_WIFI,
    AP_MODE,
    NORMAL_OPERATION,
    API_ERROR,
    SENSOR_ERROR
};

class LCDManager {
  private:
    LiquidCrystal_I2C lcd;
    uint8_t animationFrame;
    unsigned long lastAnimationUpdate;

  public:
    LCDManager(uint8_t lcd_addr, uint8_t cols, uint8_t rows);
    void init();
    void displayWelcomeScreen();
    void displayLoadingAnimation();
    void updateDisplay(LcdState state, float temp, float humid, bool motion, bool wifiConnected, bool apiConnected);
    void showWiFiConnecting(String ssid);
    void showWiFiConnected(String ip);
    void showWiFiError();
    void showAPMode(String ssid);
};

#endif // LCD_MANAGER_H
