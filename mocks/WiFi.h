#pragma once
#include "Arduino.h"
#include <string>

typedef enum {
    WIFI_STA,
    WIFI_AP
} wifi_mode_t;

typedef enum {
    WIFI_POWER_19_5dBm = 78,
    WIFI_POWER_19dBm = 76,
    WIFI_POWER_18_5dBm = 74,
    WIFI_POWER_17dBm = 68,
    WIFI_POWER_15dBm = 60,
    WIFI_POWER_13dBm = 52,
    WIFI_POWER_11dBm = 44,
    WIFI_POWER_8_5dBm = 34,
    WIFI_POWER_7dBm = 28,
    WIFI_POWER_5dBm = 20,
    WIFI_POWER_2dBm = 8,
    WIFI_POWER_MINUS_1dBm = -4,
} wifi_power_t;

enum wl_status_t {
    WL_NO_SHIELD        = 255,
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
};

class IPAddress {
public:
    IPAddress() {}
    String toString() { return "192.168.1.1"; }
};

class WiFiClass {
public:
    void mode(wifi_mode_t m) {}
    void setTxPower(wifi_power_t p) {}
    void begin(const char* ssid, const char* pass) {}
    wl_status_t status() { return WL_CONNECTED; }
    void softAP(const char* ssid) {}
    void softAPdisconnect(bool w = false) {}
    int softAPgetStationNum() { return 0; }
    IPAddress localIP() { return IPAddress(); }
    IPAddress softAPIP() { return IPAddress(); }
    void setAutoReconnect(bool r) {}
    void persistent(bool p) {}
    void setSleep(bool s) {}
    void disconnect() {}
    void reconnect() {}
};

extern WiFiClass WiFi;

class WiFiClient {
public:
    bool connected() { return false; }
};
