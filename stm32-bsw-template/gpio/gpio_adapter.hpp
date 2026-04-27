// This is the only file that includes STM32 HAL headers. Everything above this layer is HAL-agnostic. To port to a different target, replace this file.
// Pragma once is used to prevent multiple inclusions of the same file.
#pragma once

#ifndef BSW_HAL_STUB
#include "stm32xxxx_hal.h" // placeholder — user will update to their specific family

// Maps to HAL_GPIO_Init.
#define BSW_HAL_GPIO_INIT(port, halInitStruct)     HAL_GPIO_Init(port, &(halInitStruct))
// Maps to HAL_GPIO_DeInit.
#define BSW_HAL_GPIO_DEINIT(port, pin)             HAL_GPIO_DeInit(port, pin)
// Maps to HAL_GPIO_WritePin.
#define BSW_HAL_GPIO_WRITE(port, pin, state)       HAL_GPIO_WritePin(port, pin, state)
// Maps to HAL_GPIO_ReadPin.
#define BSW_HAL_GPIO_READ(port, pin)               HAL_GPIO_ReadPin(port, pin)
// Maps to HAL_GPIO_TogglePin.
#define BSW_HAL_GPIO_TOGGLE(port, pin)             HAL_GPIO_TogglePin(port, pin)
#endif // BSW_HAL_STUB

namespace Bsw {

// Translates BSW enum class values to STM32 HAL constants. Lives here because it is the adapter's job to know HAL types — nothing above this layer should.
inline void translateConfigToHal(const GpioConfig& cfg, GPIO_InitTypeDef& halInit) {
    halInit.Pin = cfg.pin;

    // GpioMode → STM32 mode constant
    switch (cfg.mode) {
        case GpioMode::Input:    halInit.Mode = GPIO_MODE_INPUT; break;
        case GpioMode::OutputPP: halInit.Mode = GPIO_MODE_OUTPUT_PP; break;
        case GpioMode::OutputOD: halInit.Mode = GPIO_MODE_OUTPUT_OD; break;
        case GpioMode::Analog:   halInit.Mode = GPIO_MODE_ANALOG; break;
    }

    // GpioPull → STM32 pull constant
    switch (cfg.pull) {
        case GpioPull::NoPull:   halInit.Pull = GPIO_NOPULL; break;
        case GpioPull::PullUp:   halInit.Pull = GPIO_PULLUP; break;
        case GpioPull::PullDown: halInit.Pull = GPIO_PULLDOWN; break;
    }

    // GpioSpeed → STM32 speed constant
    switch (cfg.speed) {
        case GpioSpeed::Low:      halInit.Speed = GPIO_SPEED_FREQ_LOW; break;
        case GpioSpeed::Medium:   halInit.Speed = GPIO_SPEED_FREQ_MEDIUM; break;
        case GpioSpeed::High:     halInit.Speed = GPIO_SPEED_FREQ_HIGH; break;
        case GpioSpeed::VeryHigh: halInit.Speed = GPIO_SPEED_FREQ_VERY_HIGH; break;
    }
}

} // namespace Bsw
