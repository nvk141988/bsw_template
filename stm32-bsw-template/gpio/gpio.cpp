// GPIO driver implementation. Calls the HAL adapter — does not call STM32 HAL directly.
#include "gpio.hpp"
#include "gpio_adapter.hpp"

namespace Bsw {

// Translates config, initialises hardware, sets _initialized. If HAL init fails this is where you'd add fault handling in a safety-critical build.
Gpio::Gpio(const GpioConfig& cfg) : _cfg(cfg), _initialized(false) {
    GPIO_InitTypeDef halInit = {};
    translateConfigToHal(_cfg, halInit);
    BSW_HAL_GPIO_INIT(_cfg.port, halInit);
    _initialized = true;
}

Status Gpio::write(uint16_t pin, GpioState state) {
    // Not initialised. Return early.
    if (!_initialized) return Status::NotInitialized;
    
    // GpioState to HAL pin state.
    auto halState = (state == GpioState::High) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    BSW_HAL_GPIO_WRITE(_cfg.port, pin, halState);
    
    return Status::Ok;
}

BswResult<GpioState> Gpio::read(uint16_t pin) {
    // Not initialised. Return early.
    if (!_initialized) return BswResult<GpioState>{Status::NotInitialized, GpioState::Low};
    
    auto halState = BSW_HAL_GPIO_READ(_cfg.port, pin);
    
    // HAL returns GPIO_PIN_SET or GPIO_PIN_RESET. Map to GpioState.
    GpioState state = (halState == GPIO_PIN_SET) ? GpioState::High : GpioState::Low;
    
    return BswResult<GpioState>{Status::Ok, state};
}

Status Gpio::toggle(uint16_t pin) {
    // Not initialised. Return early.
    if (!_initialized) return Status::NotInitialized;
    
    BSW_HAL_GPIO_TOGGLE(_cfg.port, pin);
    return Status::Ok;
}

bool Gpio::isReady() const {
    return _initialized;
}

// Releases the pin and marks the instance as unusable.
void Gpio::deinit() {
    // Not initialised. Return early.
    if (!_initialized) return;
    BSW_HAL_GPIO_DEINIT(_cfg.port, _cfg.pin);
    _initialized = false;
}

} // namespace Bsw
