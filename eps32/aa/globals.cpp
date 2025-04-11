#include "config.h"

// Define global variables
bool isWiFiConnected = false;
bool motionDetected = false;
bool isApiConnected = false;
bool isWsConnected = false;
bool light1Status = false;
bool light2Status = false;
int fanSpeed = 0;  // 0-255 for PWM control

unsigned long lastMotionTime = 0;
unsigned long lastDataSend = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastAnimationUpdate = 0;
unsigned long lastPingTime = 0;
unsigned long lastDeviceUpdate = 0;
uint8_t animationFrame = 0;
uint8_t currentDisplayPage = 0;  // For cycling through display pages

LcdState currentLcdState = STARTING;