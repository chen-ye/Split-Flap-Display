#pragma once

#include <stdint.h>
#include <vector>

class TwoWire {
public:
    void begin(int sda, int scl);
    void setClock(uint32_t freq);
    void beginTransmission(uint8_t address);
    size_t write(uint8_t data);
    uint8_t endTransmission();
    uint8_t requestFrom(uint8_t address, uint8_t quantity);
    int available();
    int read();

    // Test helpers
    void setMockResponse(uint8_t address, const std::vector<uint8_t>& data);
    std::vector<uint8_t> getWrittenData(uint8_t address);

private:
    uint8_t currentAddress;
    std::vector<uint8_t> txBuffer;
    std::vector<uint8_t> rxBuffer;
    int rxIndex;
};

extern TwoWire Wire;
