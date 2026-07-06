#pragma once
#include "Arduino.h"
#include "WiFi.h"
#include <functional>

class PubSubClient {
public:
    PubSubClient() {}
    PubSubClient(WiFiClient& client) {}
    PubSubClient& setServer(const char* server, uint16_t port) { return *this; }
    PubSubClient& setCallback(std::function<void(char*, byte*, unsigned int)> cb) { return *this; }
    bool connect(const char* id, const char* user = nullptr, const char* pass = nullptr, const char* willTopic = nullptr, uint8_t willQos = 0, bool willRetain = false, const char* willMessage = nullptr) { return true; }
    bool connected() { return true; }
    void loop() {}
    bool publish(const char* topic, const char* payload, bool retained = false) { return true; }
    bool subscribe(const char* topic) { return true; }
};
