#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <DHT.h>

class SensorManager {
  private:
    DHT dht;
    uint8_t pirPin;
    uint8_t ledPin;
    bool motionState;
    unsigned long lastMotionTime;

  public:
    SensorManager(uint8_t dhtPin, uint8_t dhtType, uint8_t pirPin, uint8_t ledPin);
    void begin();
    float readTemperature();
    float readHumidity();
    bool detectMotion();
    void updateMotionStatus(unsigned long motionTimeout);
    bool getMotionState();
};

#endif // SENSOR_MANAGER_H
