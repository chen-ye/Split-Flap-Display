#include <unity.h>
#include "SplitFlapDisplay.h"
#include "SplitFlapModule.h"
#include "Arduino.h"
#include "Wire.h"

#include "JsonSettings.h"

// Mock settings values
std::vector<uint8_t> addresses = {0x20, 0x21};
std::vector<int> offsets = {0, 0};
int stepsPerRot = 2048;
int displayOffset = 0;
int magnetPosition = 730;
float maxVel = 15.0f;
int charSetSize = 37;
String customCharset = "";
int sdaPin = 8;
int sclPin = 9;

JsonSettings* settings;
SplitFlapDisplay* display;

void setUp(void) {
    settings = new JsonSettings("config", {
        {"moduleCount", JsonSetting(2)},
        {"moduleAddresses", JsonSetting(std::vector<int>{0x20, 0x21})},
        {"moduleOffsets", JsonSetting(std::vector<int>{0, 0})},
        {"displayOffset", JsonSetting(0)},
        {"magnetPosition", JsonSetting(730)},
        {"stepsPerRot", JsonSetting(2048)},
        {"maxVel", JsonSetting(15.0f)},
        {"charset", JsonSetting(37)},
        {"custom_charset", JsonSetting("")},
        {"sdaPin", JsonSetting(8)},
        {"sclPin", JsonSetting(9)},
        {"mode", JsonSetting(0)}
    });
    display = new SplitFlapDisplay(*settings);
    display->init();
}

void tearDown(void) {
    delete display;
    delete settings;
}

void test_initialization(void) {
    TEST_ASSERT_EQUAL(2, display->getNumModules());
    TEST_ASSERT_EQUAL(37, display->getCharsetSize());
}

void test_char_mapping(void) {
    // Standard charset: " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    // ' ' is at index 0
    // 'A' is at index 1

    SplitFlapModule module(0x20, 2048, 0, 730, 37, "");
    module.init();

    // Calculate expected position for 'A'
    // 2048 steps / 37 chars = ~55.35 steps per char
    // 'A' is index 1 => ~55 steps

    int posA = module.getCharPosition('A');
    TEST_ASSERT_INT_WITHIN(1, 55, posA);

    int posSpace = module.getCharPosition(' ');
    TEST_ASSERT_EQUAL(0, posSpace);
}

void test_string_padding_centering(void) {
    // We can't easily test the internal padding logic since it's inside writeString and doesn't return the string.
    // But we can verify that writeString doesn't crash.
    display->writeString("A");

    // Verify motors would move (mocking Wire would allow checking written data)
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_initialization);
    RUN_TEST(test_char_mapping);
    RUN_TEST(test_string_padding_centering);
    UNITY_END();

    return 0;
}
