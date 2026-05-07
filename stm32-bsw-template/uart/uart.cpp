// UART driver implementation. Calls the HAL adapter — does not call STM32 HAL directly.
#include "uart.hpp"
#include "uart_adapter.hpp"

namespace Bsw {

// Initialises HAL, resets ring buffer state, clears busy flag.
Uart::Uart(const UartConfig& cfg)
    : _cfg(cfg), _initialized(false), _txBusy(false), _txHead(0), _txTail(0)
{
    auto halInit = buildHalUartInit(cfg);
    _cfg.handle->Init = halInit;
    if (HAL_UART_Init(_cfg.handle) == HAL_OK) {
        _initialized = true;
    }
}

Status Uart::send(const uint8_t* data, uint16_t len, uint32_t timeout_ms) {
    // Not initialised. Return early.
    if (!_initialized) return Status::NotInitialized;

    auto result = BSW_HAL_UART_TX(_cfg.handle, data, len, timeout_ms);

    // HAL enforces the timeout. Map each return value explicitly — no catch-all.
    switch (result) {
        case HAL_OK:      return Status::Ok;
        case HAL_TIMEOUT: return Status::UartTimeout;
        case HAL_BUSY:    return Status::UartBusy;
        default:          return Status::Error;
    }
}

Status Uart::receive(uint8_t* data, uint16_t len, uint32_t timeout_ms) {
    // Not initialised. Return early.
    if (!_initialized) return Status::NotInitialized;

    auto result = BSW_HAL_UART_RX(_cfg.handle, data, len, timeout_ms);

    // HAL enforces the timeout. Map each return value explicitly — no catch-all.
    switch (result) {
        case HAL_OK:      return Status::Ok;
        case HAL_TIMEOUT: return Status::UartTimeout;
        case HAL_BUSY:    return Status::UartBusy;
        default:          return Status::Error;
    }
}

Status Uart::send_IT(const uint8_t* data, uint16_t len) {
    // Not initialised. Return early.
    if (!_initialized) return Status::NotInitialized;

    // Copy into TX ring buffer. send_IT() returns immediately after this.
    for (uint16_t i = 0; i < len; i++) {
        _txBuf[_txTail] = data[i];
        _txTail = static_cast<uint16_t>((_txTail + 1) % BSW_UART_TX_BUF_SIZE);
    }

    // Buffer was empty. Start transmission now.
    if (!_txBusy) {
        BSW_HAL_UART_TX_IT(_cfg.handle, &_txBuf[_txHead], 1);
        _txBusy = true;
    }

    return Status::Ok;
}

Status Uart::startReceive_IT() {
    // Not initialised. Return early.
    if (!_initialized) return Status::NotInitialized;

    BSW_HAL_UART_RX_IT(_cfg.handle, _cfg.rxBuf, _cfg.rxBufSize);
    return Status::Ok;
}

BswResult<uint16_t> Uart::readRxBuffer(uint8_t* out, uint16_t maxLen) {
    // Not initialised. Return early.
    if (!_initialized) return {Status::NotInitialized, 0};

    uint16_t bytesToRead = (maxLen < _cfg.rxBufSize) ? maxLen : _cfg.rxBufSize;
    for (uint16_t i = 0; i < bytesToRead; i++) {
        out[i] = _cfg.rxBuf[i];
    }

    return {Status::Ok, bytesToRead};
}

void Uart::irqHandler() {
    // TX complete. Feed next byte or clear busy flag.
    _txHead = static_cast<uint16_t>((_txHead + 1) % BSW_UART_TX_BUF_SIZE);

    if (_txHead != _txTail) {
        BSW_HAL_UART_TX_IT(_cfg.handle, &_txBuf[_txHead], 1);
    } else {
        _txBusy = false;
        if (_cfg.onTxComplete) _cfg.onTxComplete();
    }

    // RX complete. Notify caller and re-arm.
    if (_cfg.onRxData) _cfg.onRxData(_cfg.rxBuf, _cfg.rxBufSize);
    BSW_HAL_UART_RX_IT(_cfg.handle, _cfg.rxBuf, _cfg.rxBufSize);
}

bool Uart::isReady() const {
    return _initialized;
}

// Releases UART peripheral and marks instance as unusable.
void Uart::deinit() {
    BSW_HAL_UART_DEINIT(_cfg.handle);
    _initialized = false;
}

} // namespace Bsw
