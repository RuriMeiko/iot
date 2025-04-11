#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <Arduino.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// WebSocket function prototypes
void setupWebSocket();
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
void sendDataToServer();
void updateDeviceStatus();
void handleDeviceControl(const JsonDocument& doc);

// External reference to the WebSocket client
extern WebSocketsClient webSocket;

#endif // WEBSOCKET_HANDLER_H