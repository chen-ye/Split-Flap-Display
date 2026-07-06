#ifndef NATIVE_TEST
#include "BLELogger.h"

#ifdef ENABLE_BLE_LOG

// Nordic UART Service (NUS) UUIDs – widely supported by BLE terminal apps
// (nRF Connect, Serial Bluetooth Terminal, LightBlue, etc.)
#define NUS_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define NUS_RX_UUID      "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // Central → Peripheral
#define NUS_TX_UUID      "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // Peripheral → Central (notify)

BLELogger Logger;

// ── Server connection callback ────────────────────────────────────────────────
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer *server, NimBLEConnInfo &info) override {
        Serial.printf("[BLE] Central connected: %s\n", info.getAddress().toString().c_str());
        // Stop advertising while connected (single-central logger)
        NimBLEDevice::getAdvertising()->stop();
    }
    void onDisconnect(NimBLEServer *server, NimBLEConnInfo &info, int reason) override {
        Serial.printf("[BLE] Central disconnected (reason %d) – re-advertising\n", reason);
        NimBLEDevice::getAdvertising()->start();
    }
};

static ServerCallbacks serverCbs;

// ── BLELogger::begin ──────────────────────────────────────────────────────────
void BLELogger::begin(const char *deviceName) {
    NimBLEDevice::init(deviceName);
    NimBLEDevice::setPower(3);  // 3 dBm – enough for close-range debugging

    NimBLEServer *server = NimBLEDevice::createServer();
    server->setCallbacks(&serverCbs);

    // Nordic UART Service
    NimBLEService *nus = server->createService(NUS_SERVICE_UUID);

    _txChar = nus->createCharacteristic(NUS_TX_UUID, NIMBLE_PROPERTY::NOTIFY);

    // RX characteristic required by NUS spec; we don't act on incoming data
    nus->createCharacteristic(NUS_RX_UUID,
                              NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);

    nus->start();

    NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(NUS_SERVICE_UUID);
    adv->setName(deviceName);
    adv->start();

    _ready = true;
    Serial.printf("[BLE] Logger advertising as \"%s\"\n", deviceName);
}

// ── BLELogger::isConnected ────────────────────────────────────────────────────
bool BLELogger::isConnected() const {
    return _ready && NimBLEDevice::getServer() != nullptr &&
           NimBLEDevice::getServer()->getConnectedCount() > 0;
}

// ── BLELogger::_flush ─────────────────────────────────────────────────────────
void BLELogger::_flush() {
    if (_lineLen == 0) return;

    // 1. Always write to USB Serial first (never fails)
    Serial.write(reinterpret_cast<const uint8_t *>(_lineBuf), _lineLen);

    // 2. Send to BLE central if one is connected (fire-and-forget notify)
    //    NimBLE limits a single notify to MTU-3 bytes (default 20B negotiated
    //    up to 512B). Chunk if the line is longer than the negotiated MTU.
    if (isConnected() && _txChar != nullptr) {
        const uint8_t *src = reinterpret_cast<const uint8_t *>(_lineBuf);
        size_t remaining   = _lineLen;
        uint16_t mtu       = NimBLEDevice::getServer()
                                 ->getPeerMTU(NimBLEDevice::getServer()
                                                  ->getConnId(0)) - 3;
        if (mtu == 0 || mtu > 512) mtu = 20;  // safe fallback

        while (remaining > 0) {
            size_t chunk = (remaining < mtu) ? remaining : mtu;
            _txChar->setValue(src, chunk);
            _txChar->notify();
            src       += chunk;
            remaining -= chunk;
        }
    }

    _lineLen = 0;
}

// ── BLELogger::write ──────────────────────────────────────────────────────────
size_t BLELogger::write(uint8_t c) {
    if (_lineLen < LINE_BUF_SIZE - 1) {
        _lineBuf[_lineLen++] = static_cast<char>(c);
    }
    // Flush on newline or when the buffer is nearly full
    if (c == '\n' || _lineLen >= LINE_BUF_SIZE - 1) {
        _flush();
    }
    return 1;
}

size_t BLELogger::write(const uint8_t *buffer, size_t size) {
    // Route through single-char write so newline detection always fires
    for (size_t i = 0; i < size; i++) {
        write(buffer[i]);
    }
    return size;
}

#else  // ── ENABLE_BLE_LOG not defined ─────────────────────────────────────────

SerialLogger Logger;

#endif  // ENABLE_BLE_LOG
#endif  // !NATIVE_TEST
