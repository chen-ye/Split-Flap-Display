#pragma once
#include "Arduino.h"
#include <string>

class File {
public:
    operator bool() const { return false; }
    size_t size() { return 0; }
    size_t readBytes(char* buffer, size_t length) { return 0; }
    void close() {}
    const char* name() { return ""; }
    bool isDirectory() { return false; }
    File openNextFile() { return File(); }
};

class LittleFSClass {
public:
    bool begin() { return true; }
    void end() {}
    File open(const char* path, const char* mode = "r") { return File(); }
    operator void*() { return this; }
};

extern LittleFSClass LittleFS;
