#pragma once

#include "Arduino.h"
#include <map>

class Preferences {
public:
    bool begin(const char* name, bool readOnly = false) { return true; }
    void end() {}
    bool clear() {
        storage.clear();
        return true;
    }

    String getString(const char* key, String defaultValue = "") {
        if (storage.count(key)) return storage[key];
        return defaultValue;
    }
    int getInt(const char* key, int defaultValue = 0) {
        if (storage.count(key)) return storage[key].toInt();
        return defaultValue;
    }
    float getFloat(const char* key, float defaultValue = 0.0f) {
        if (storage.count(key)) {
            try {
                return std::stof(storage[key]);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    size_t putString(const char* key, String value) {
        storage[key] = value;
        return 1;
    }
    size_t putInt(const char* key, int value) {
        storage[key] = String(value);
        return 1;
    }
    size_t putFloat(const char* key, float value) {
        storage[key] = String(value);
        return 1;
    }

private:
    std::map<String, String> storage;
};
