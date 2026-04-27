#pragma once
#include <cstdint>

// All BSW types live in the Bsw namespace. Keeps the global scope clean.
namespace Bsw {

// Upper nibble identifies the peripheral family. Lower nibble is the specific error. Makes log parsing straightforward.
enum class Status : uint8_t {
    Ok             = 0x00,
    Error          = 0x01,
    NotInitialized = 0x02,

    // UART
    UartTimeout    = 0x10,
    UartBusy       = 0x11,

    // SPI
    SpiTimeout     = 0x20,
    SpiBusy        = 0x21,

    // I2C
    I2cTimeout     = 0x30,
    I2cNack        = 0x31,

    // ADC
    AdcNotReady    = 0x40,

    // PWM
    PwmInvalidDuty = 0x50
};

// Used when a function needs to return both a status and a value. Caller uses structured bindings: auto [status, value] = driver.read();
template<typename T>
struct BswResult {
    Status status;
    T      value;
};

} // namespace Bsw
