#include "Wire.h"
#include <map>

TwoWire Wire;

// Mock storage for responses
std::map<uint8_t, std::vector<uint8_t>> mockResponses;
std::map<uint8_t, std::vector<uint8_t>> writtenData;

void TwoWire::begin(int sda, int scl) {}
void TwoWire::setClock(uint32_t freq) {}

void TwoWire::beginTransmission(uint8_t address) {
    currentAddress = address;
    txBuffer.clear();
}

size_t TwoWire::write(uint8_t data) {
    txBuffer.push_back(data);
    return 1;
}

uint8_t TwoWire::endTransmission() {
    writtenData[currentAddress] = txBuffer;
    return 0; // Success
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity) {
    if (mockResponses.count(address)) {
        rxBuffer = mockResponses[address];
        rxIndex = 0;
        return rxBuffer.size();
    }
    return 0;
}

int TwoWire::available() {
    return rxBuffer.size() - rxIndex;
}

int TwoWire::read() {
    if (rxIndex < rxBuffer.size()) {
        return rxBuffer[rxIndex++];
    }
    return -1;
}

void TwoWire::setMockResponse(uint8_t address, const std::vector<uint8_t>& data) {
    mockResponses[address] = data;
}

std::vector<uint8_t> TwoWire::getWrittenData(uint8_t address) {
    return writtenData[address];
}
