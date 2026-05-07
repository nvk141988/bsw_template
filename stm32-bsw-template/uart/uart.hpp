// uart.hpp does not include uart_adapter.hpp — the adapter is an implementation detail.
#pragma once
#include <cstdint>
#include "../bsw_types.hpp"
#include "../bsw_config.hpp"

// Forward declaration to avoid including HAL headers.
struct UART_HandleTypeDef;

namespace Bsw {

// All UART configuration in one place. rxBuf and rxBufSize are caller-supplied —
// the driver does not allocate memory. onRxData and onTxComplete are optional —
// set to nullptr if unused.
struct UartConfig {
    // STM32 HAL UART handle. e.g. &huart2.
    UART_HandleTypeDef* handle;
    uint32_t            baudRate;
    UartWordLength      wordLength;
    UartStopBits        stopBits;
    UartParity          parity;
    // Caller supplies this buffer. Must remain valid for the lifetime of the Uart instance.
    uint8_t*            rxBuf;
    uint16_t            rxBufSize;
    // Called from irqHandler() when new data arrives. nullptr if not needed.
    // Runs in interrupt context — keep it short.
    void (*onRxData)(uint8_t*, uint16_t);
    // Called from irqHandler() when TX is done. nullptr if not needed.
    // Runs in interrupt context.
    void (*onTxComplete)();
};

// One instance per UART peripheral. Constructor initialises hardware.
// Check isReady() if init can fail silently.
class Uart {
public:
    // Constructor handles init. Sets _initialized on success.
    explicit Uart(const UartConfig& cfg);

    // Blocking — waits until complete or timeout.
    Status send(const uint8_t* data, uint16_t len, uint32_t timeout_ms);
    Status receive(uint8_t* data, uint16_t len, uint32_t timeout_ms);

    // Copies data into internal TX ring buffer.
    // Kicks transmission immediately if not already busy. Returns immediately.
    Status send_IT(const uint8_t* data, uint16_t len);

    // Arms HAL to receive into caller-supplied RX buffer. Call once after construction.
    Status startReceive_IT();

    // Reads available bytes from RX buffer. Returns bytes read. Non-blocking.
    BswResult<uint16_t> readRxBuffer(uint8_t* out, uint16_t maxLen);

    // Call this from the STM32 HAL UART IRQ callback for this instance.
    // Feeds TX ring buffer and drains RX.
    void irqHandler();

    bool isReady() const;
    void deinit();

private:
    UartConfig _cfg;
    bool       _initialized;
    // True while a HAL IT transmission is in progress. Prevents overlapping transfers.
    bool       _txBusy;

    // Internal TX ring buffer. Caller data is copied here. Size set in bsw_config.hpp.
    uint8_t  _txBuf[BSW_UART_TX_BUF_SIZE];
    // Head: next byte to read. Tail: next byte to write.
    uint16_t _txHead;
    uint16_t _txTail;
};

} // namespace Bsw
