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

#define PROGMEM
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

class __FlashStringHelper;

class Print {
public:
    virtual size_t write(uint8_t) = 0;
    virtual size_t write(const uint8_t *buffer, size_t size) {
        size_t n = 0;
        while (size--) {
            if (write(*buffer++)) n++;
            else break;
        }
        return n;
    }
};

class Printable {
public:
    virtual size_t printTo(Print& p) const = 0;
};

class Stream {
public:
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;
    virtual size_t readBytes(char *buffer, size_t length) {
        size_t count = 0;
        while (count < length) {
            int c = read();
            if (c < 0) break;
            *buffer++ = (char)c;
            count++;
        }
        return count;
    }
};

typedef uint8_t byte;
typedef bool boolean;

class String : public std::string {
public:
    String(const char* s) : std::string(s ? s : "") {}
    String(std::string s) : std::string(s) {}
    String(int i) : std::string(std::to_string(i)) {}
    String() : std::string() {}

    int toInt() const { return this->empty() ? 0 : std::stoi(*this); }
    String substring(int start, int end) const { return this->substr(start, end - start); }
    String substring(int start) const { return this->substr(start); }
    void replace(String s1, String s2) {
        size_t pos = 0;
        while ((pos = this->find(s1, pos)) != std::string::npos) {
             std::string::replace(pos, s1.length(), s2);
             pos += s2.length();
        }
    }
    bool isEmpty() const { return this->empty(); }
    bool concat(const char* s) { this->append(s ? s : ""); return true; }
    bool concat(const String& s) { this->append(s); return true; }
    bool endsWith(String suffix) const {
        if (this->length() >= suffix.length()) {
            return (0 == this->compare(this->length() - suffix.length(), suffix.length(), suffix));
        }
        return false;
    }
    void remove(unsigned int index, unsigned int count = 1) {
        this->erase(index, count);
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
    template<typename... Args>
    void printf(const char* format, Args... args) {
        ::printf(format, args...);
    }
};

extern SerialMock Serial;

class EspClass {
public:
    void restart() { exit(0); }
};
extern EspClass ESP;

#include <time.h>
inline bool getLocalTime(struct tm *timeinfo) {
    time_t rawtime;
    time(&rawtime);
    struct tm *info = localtime(&rawtime);
    if (info == nullptr) return false;
    *timeinfo = *info;
    return true;
}

inline void configTzTime(const char* tz, const char* server1, const char* server2 = nullptr, const char* server3 = nullptr) {}

unsigned long millis();
unsigned long micros();
void delay(unsigned long ms);
void yield();
long random(long min, long max);

