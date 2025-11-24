#include "Arduino.h"
#include <chrono>

SerialMock Serial;

unsigned long millis() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

unsigned long micros() {
    using namespace std::chrono;
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

void delay(unsigned long ms) {
    // No-op for tests to run fast, or could use sleep if needed
}

long random(long min, long max) {
    return min + (std::rand() % (max - min));
}

void yield() {}
