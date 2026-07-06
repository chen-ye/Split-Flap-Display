#include "JsonSettings.h"

#include <ArduinoJson.h>
#include <sstream>

// Helper to safely begin preferences namespace with fallback
bool safePreferencesBegin(Preferences &prefs, const char *ns, bool readOnly) {
    if (prefs.begin(ns, readOnly)) {
        return true;
    }
    return prefs.begin(ns, false);
}

String JsonSettings::getString(const char *key) {
    safePreferencesBegin(preferences, name, true);
    String value = preferences.getString(key, this->find(key).strDefault);
    preferences.end();
    return value;
}

int JsonSettings::getInt(const char *key) {
    safePreferencesBegin(preferences, name, true);
    int value = preferences.getInt(key, this->find(key).intDefault);
    preferences.end();
    return value;
}

float JsonSettings::getFloat(const char *key) {
    safePreferencesBegin(preferences, name, true);
    float value = preferences.getFloat(key, this->find(key).floatDefault);
    preferences.end();
    return value;
}

std::vector<int> JsonSettings::getIntVector(const char *key) {
    safePreferencesBegin(preferences, name, true);
    String value = preferences.getString(key, this->find(key).strDefault);
    preferences.end();

    std::vector<int> intVector;
    std::istringstream stream(value.c_str());
    std::string token;
    while (std::getline(stream, token, ',')) {
        try {
            intVector.push_back(std::stoi(token));
        } catch (const std::invalid_argument &) {
            throw std::runtime_error("Invalid CSV: Non-integer value found");
        } catch (const std::out_of_range &) {
            throw std::runtime_error("Invalid CSV: Integer value out of range");
        }
    }
    return intVector;
}

void JsonSettings::putString(const char *key, String value) {
    safePreferencesBegin(preferences, name, false);
    preferences.putString(key, value);
    preferences.end();
}

void JsonSettings::putInt(const char *key, int value) {
    safePreferencesBegin(preferences, name, false);
    preferences.putInt(key, value);
    preferences.end();
}

void JsonSettings::putFloat(const char *key, float value) {
    safePreferencesBegin(preferences, name, false);
    preferences.putFloat(key, value);
    preferences.end();
}

void JsonSettings::putIntVector(const char *key, std::vector<int> value) {
    std::ostringstream stream;
    for (size_t i = 0; i < value.size(); ++i) {
        stream << value[i];
        if (i < value.size() - 1) {
            stream << ",";
        }
    }
    putString(key, stream.str().c_str());
}

JsonDocument JsonSettings::toJson() {
    JsonDocument settings;

    safePreferencesBegin(preferences, name, true);

    for (const auto &pair : map) {
        const String &key = pair.first;
        const JsonSetting &setting = pair.second;

        switch (setting.type) {
            case JsonSettingType::JST_STR:
            case JsonSettingType::JST_INT_VECTOR:
                settings[key] = preferences.getString(key.c_str(), setting.strDefault);
                break;
            case JsonSettingType::JST_INT: settings[key] = preferences.getInt(key.c_str(), setting.intDefault); break;
            case JsonSettingType::JST_FLOAT:
                settings[key] = preferences.getFloat(key.c_str(), setting.floatDefault);
                break;
        }
    }

    preferences.end();
    return settings;
}

bool JsonSettings::fromJson(JsonVariant settings) {
    safePreferencesBegin(preferences, name, false);

    for (JsonPair kv : settings.as<JsonObject>()) {
        const char *key = kv.key().c_str();
        JsonSetting setting = this->find(key);

        if (! setting.validate(kv.value().as<String>())) {
            lastValidationError = setting.getLastValidationError();
            lastValidationKey = String(key);
            return false;
        }

        switch (setting.type) {
            case JsonSettingType::JST_INT_VECTOR:
            case JsonSettingType::JST_STR: preferences.putString(key, kv.value().as<String>()); break;
            case JsonSettingType::JST_INT: preferences.putInt(key, kv.value().as<int>()); break;
            case JsonSettingType::JST_FLOAT: preferences.putFloat(key, kv.value().as<float>()); break;
        }
    }

    preferences.end();

    return true;
}

bool JsonSettings::reset() {
    safePreferencesBegin(preferences, "config", false);
    preferences.clear();
    preferences.end();

    return fromJson(toJson());
}

JsonSetting JsonSettings::find(const char *key) {
    auto it = this->map.find(key);
    if (it == this->map.end()) {
        throw std::runtime_error("Key not found in settings map");
    }
    return it->second;
}
