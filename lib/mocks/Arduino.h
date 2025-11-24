#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <cmath>

#define OUTPUT 0x01
#define INPUT 0x00
#define LOW 0x0
#define HIGH 0x1

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

typedef uint8_t byte;
typedef bool boolean;

class String : public std::string {
public:
    String(const char* s) : std::string(s) {}
    String(std::string s) : std::string(s) {}
    String(int i) : std::string(std::to_string(i)) {}
    String() : std::string() {}

    int toInt() const { return std::stoi(*this); }
    String substring(int start, int end) const { return this->substr(start, end - start); }
    String substring(int start) const { return this->substr(start); }
    void replace(String s1, String s2) {
        size_t pos = 0;
        while ((pos = this->find(s1, pos)) != std::string::npos) {
             std::string::replace(pos, s1.length(), s2);
             pos += s2.length();
        }
    }
};

class SerialMock {
public:
    void begin(int baud) {}
    void print(const char* s) { std::cout << s; }
    void print(int i) { std::cout << i; }
    void print(String s) { std::cout << s; }
    void println(const char* s) { std::cout << s << std::endl; }
    void println(int i) { std::cout << i << std::endl; }
    void println(String s) { std::cout << s << std::endl; }
    void println() { std::cout << std::endl; }
};

extern SerialMock Serial;

unsigned long millis();
unsigned long micros();
void delay(unsigned long ms);
void yield();
long random(long min, long max);
