/*
 * File: sensors.cpp
 * Description: Implementation of sensor management functions
 */

#include "sensors.h"

// Initialize DHT object
DHT dht(DHTPIN, DHTTYPE);

// Variables to store sensor states and measured values
boolean motionDetected = false;        // Whether motion is detected or not
unsigned long motionStartTime = 0;     // Timestamp when motion was first detected
float currentTemperature = 0;          // Current temperature
float currentHumidity = 0;             // Current humidity

void setupSensors() {
  // Configure motion sensor pin (INPUT)
  pinMode(MOTION_PIN, INPUT);
  
  // Configure LED pin (OUTPUT) and turn off LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize DHT sensor
  dht.begin();
  
  Serial.println("DHT11 and motion sensors initialized");
}

void handleSensors() {
  // === Handle motion sensor ===
  int motionState = digitalRead(MOTION_PIN);
  
  // If motion is detected and not already recorded
  if (motionState == HIGH && !motionDetected) {
    motionDetected = true;
    motionStartTime = millis();
    digitalWrite(LED_PIN, HIGH);  // Turn on LED
    
    Serial.println("Motion detected! LED turned on");
  }
  
  // If motion was detected and 5 seconds have passed
  if (motionDetected && (millis() - motionStartTime > 5000)) {
    motionDetected = false;
    digitalWrite(LED_PIN, LOW);  // Turn off LED
    
    Serial.println("5 seconds passed, LED turned off");
  }
  
  // === Read data from DHT11 ===
  // DHT11 needs time to read accurately, read every 2 seconds
  static unsigned long lastDHTRead = 0;
  
  if (millis() - lastDHTRead > 2000) {
    lastDHTRead = millis();
    
    // Read humidity
    float newHumidity = dht.readHumidity();
    // Read temperature (Celsius)
    float newTemperature = dht.readTemperature();
    
    // Check if the readings are valid
    if (!isnan(newHumidity) && !isnan(newTemperature)) {
      currentHumidity = newHumidity;
      currentTemperature = newTemperature;
      
      Serial.print("Temperature: ");
      Serial.print(currentTemperature);
      Serial.print("°C, Humidity: ");
      Serial.print(currentHumidity);
      Serial.println("%");
    } else {
      Serial.println("Cannot read data from DHT sensor!");
    }
  }
}

float getTemperature() {
  return currentTemperature;
}

float getHumidity() {
  return currentHumidity;
}

bool isMotionDetected() {
  return motionDetected;
}