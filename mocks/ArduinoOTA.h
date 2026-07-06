#pragma once
#include "Arduino.h"

typedef enum {
    U_FLASH = 0,
    U_LITTLEFS = 1
} ota_command_t;

typedef enum {
    OTA_AUTH_ERROR,
    OTA_BEGIN_ERROR,
    OTA_CONNECT_ERROR,
    OTA_RECEIVE_ERROR,
    OTA_END_ERROR
} ota_error_t;

class ArduinoOTAMock {
public:
    void setHostname(const char* h) {}
    void setPassword(const char* p) {}
    ota_command_t getCommand() { return U_FLASH; }
    ArduinoOTAMock& onStart(void (*fn)()) { return *this; }
    ArduinoOTAMock& onEnd(void (*fn)()) { return *this; }
    ArduinoOTAMock& onProgress(void (*fn)(unsigned int, unsigned int)) { return *this; }
    ArduinoOTAMock& onError(void (*fn)(ota_error_t)) { return *this; }
    void begin() {}
    void handle() {}
};

extern ArduinoOTAMock ArduinoOTA;
