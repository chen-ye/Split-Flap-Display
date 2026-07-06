#pragma once
#include "Arduino.h"
#include <ArduinoJson.h>
#include <functional>

class AsyncWebServerRequest {
public:
    int method() { return 0; }
    String methodToString() { return "GET"; }
    String url() { return "/"; }
    void redirect(String url) {}
    void send(int code, String type = "", String body = "") {}
};

class AsyncWebHandler {};

class AsyncWebServer {
public:
    AsyncWebServer(int port) {}
    void begin() {}
    void on(const char* uri, int method, std::function<void(AsyncWebServerRequest*)> fn) {}
    void serveStatic(const char* uri, void* fs, const char* path, const char* cache = "") {}
    void addHandler(AsyncWebHandler* handler) {}
    void onNotFound(void (*fn)(AsyncWebServerRequest*)) {}
};

#define HTTP_GET 1
#define HTTP_POST 2

// Mock AsyncCallbackJsonWebHandler
class AsyncCallbackJsonWebHandler : public AsyncWebHandler {
public:
    AsyncCallbackJsonWebHandler(const char* uri, std::function<void(AsyncWebServerRequest*, JsonVariant&)> fn) {}
};
