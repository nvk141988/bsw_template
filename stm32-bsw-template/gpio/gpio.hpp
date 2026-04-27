// gpio.hpp does not include gpio_adapter.hpp. The adapter is an implementation detail — it belongs in gpio.cpp only.
#pragma once
#include <cstdint>
#include "../bsw_types.hpp"

// Forward declaration to avoid including HAL headers
struct GPIO_TypeDef;

namespace Bsw {

// Pin mode. enum class prevents implicit conversion to int.
enum class GpioMode  : uint8_t { Input, OutputPP, OutputOD, Analog };

// Pin pull configuration. enum class prevents implicit conversion to int.
enum class GpioPull  : uint8_t { NoPull, PullUp, PullDown };

// Pin speed. enum class prevents implicit conversion to int.
enum class GpioSpeed : uint8_t { Low, Medium, High, VeryHigh };

// Pin state. enum class prevents implicit conversion to int.
enum class GpioState : uint8_t { Low = 0, High = 1 };

// All pin configuration in one place. Passed to the constructor. Not modified after init.
struct GpioConfig {
    // STM32 port register base address. e.g. GPIOA, GPIOB.
    GPIO_TypeDef* port;
    uint16_t      pin;
    GpioMode      mode;
    GpioPull      pull;
    GpioSpeed     speed;
};

// One instance per configured pin or pin group. Constructor initialises the hardware. Check isReady() if init can fail silently.
class Gpio {
public:
    // Constructor handles init. No separate init() call needed. Sets _initialized on success.
    explicit Gpio(const GpioConfig& cfg);

    // Drive the pin high or low.
    Status               write(uint16_t pin, GpioState state);
    
    // Sample the pin. Returns state and status together.
    BswResult<GpioState> read(uint16_t pin);
    
    // Flip the current output state.
    Status               toggle(uint16_t pin);
    
    // Returns true if constructor succeeded and deinit has not been called.
    bool                 isReady() const;
    
    // Release the pin. Sets _initialized to false.
    void                 deinit();

private:
    GpioConfig _cfg;
    
    // Set in constructor. Guards every method. Prevents use before init.
    bool       _initialized;
};

} // namespace Bsw
