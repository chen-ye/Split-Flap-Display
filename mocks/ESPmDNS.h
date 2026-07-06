#pragma once
#include "Arduino.h"

class MDNSClass {
public:
    bool begin(const char* host) { return true; }
    void end() {}
};

extern MDNSClass MDNS;
