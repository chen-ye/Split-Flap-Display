#pragma once
#ifndef NATIVE_TEST
#include <Arduino.h>
#include <Print.h>

#ifdef ENABLE_BLE_LOG

#include <NimBLEDevice.h>

/**
 * BLELogger – dual-output logger (USB Serial + BLE Nordic UART Service).
 *
 * Extends Print so the full Arduino print/println/printf API works out of
 * the box. Output is line-buffered: each newline (or a full 512-byte buffer)
 * triggers a flush that writes to both Serial and the connected BLE central.
 *
 * BLE writes are fire-and-forget (notify). If no central is connected, or the
 * notify queue is full, the message is silently dropped on the BLE side but
 * still written to USB Serial – so logging never blocks the main loop.
 *
 * Usage:
 *   Logger.begin("SplitFlap");          // in setup()
 *   Logger.println("Hello, world!");    // anywhere
 *   Logger.printf("Step: %d\n", step); // printf-style
 */
class BLELogger : public Print {
public:
    /**
     * Initialise NimBLE, create the NUS service, and start advertising.
     * Call once from setup() before any log output.
     */
    void begin(const char *deviceName = "SplitFlap-Log");

    /** True when at least one BLE central is connected. */
    bool isConnected() const;

    // ── Print interface ──────────────────────────────────────────────────────
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    using Print::write; // expose write(const char*) etc.

private:
    NimBLECharacteristic *_txChar = nullptr;
    bool                  _ready  = false;

    static constexpr size_t LINE_BUF_SIZE = 512;
    char   _lineBuf[LINE_BUF_SIZE];
    size_t _lineLen = 0;

    void _flush();
};

extern BLELogger Logger;

#else  // ── ENABLE_BLE_LOG not defined ─────────────────────────────────────────

/**
 * No-overhead stub used when BLE logging is disabled.
 * All output goes straight to Serial; no BLE stack is initialised.
 */
class SerialLogger : public Print {
public:
    void begin(const char * = nullptr) {}
    bool isConnected() const { return false; }
    size_t write(uint8_t c) override               { return Serial.write(c); }
    size_t write(const uint8_t *buf, size_t n) override { return Serial.write(buf, n); }
    using Print::write;
};

extern SerialLogger Logger;

#endif  // ENABLE_BLE_LOG
#endif  // !NATIVE_TEST
