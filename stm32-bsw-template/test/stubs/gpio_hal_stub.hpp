// HAL stub for host-side unit testing. No hardware, no toolchain. Replaces gpio_adapter.hpp at compile time with -DBSW_HAL_STUB. Lets us test driver logic on any machine.
#pragma once

#include <cstdint>

// Empty struct. The driver only uses the pointer as an identifier.
struct GPIO_TypeDef {};

// Minimal fields. Enough for the adapter translation function to compile.
struct GPIO_InitTypeDef {
    uint32_t Pin;
    uint32_t Mode;
    uint32_t Pull;
    uint32_t Speed;
};

enum GPIO_PinState {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET = 1
};

// STM32 uses bitmasks for pin numbers. Replicate the same values so test code matches production code.
constexpr uint16_t GPIO_PIN_0  = 0x0001;
constexpr uint16_t GPIO_PIN_1  = 0x0002;
constexpr uint16_t GPIO_PIN_2  = 0x0004;
constexpr uint16_t GPIO_PIN_3  = 0x0008;
constexpr uint16_t GPIO_PIN_4  = 0x0010;
constexpr uint16_t GPIO_PIN_5  = 0x0020;
constexpr uint16_t GPIO_PIN_6  = 0x0040;
constexpr uint16_t GPIO_PIN_7  = 0x0080;
constexpr uint16_t GPIO_PIN_8  = 0x0100;
constexpr uint16_t GPIO_PIN_9  = 0x0200;
constexpr uint16_t GPIO_PIN_10 = 0x0400;
constexpr uint16_t GPIO_PIN_11 = 0x0800;
constexpr uint16_t GPIO_PIN_12 = 0x1000;
constexpr uint16_t GPIO_PIN_13 = 0x2000;
constexpr uint16_t GPIO_PIN_14 = 0x4000;
constexpr uint16_t GPIO_PIN_15 = 0x8000;

#define GPIO_MODE_INPUT     0x00
#define GPIO_MODE_OUTPUT_PP 0x01
#define GPIO_MODE_OUTPUT_OD 0x11
#define GPIO_MODE_ANALOG    0x03

#define GPIO_NOPULL   0x00
#define GPIO_PULLUP   0x01
#define GPIO_PULLDOWN 0x02

#define GPIO_SPEED_FREQ_LOW       0x00
#define GPIO_SPEED_FREQ_MEDIUM    0x01
#define GPIO_SPEED_FREQ_HIGH      0x02
#define GPIO_SPEED_FREQ_VERY_HIGH 0x03

// These globals record the last HAL call. Tests assert against them to verify driver behaviour.
inline GPIO_TypeDef* bsw_stub_last_port = nullptr;
inline uint16_t      bsw_stub_last_pin  = 0;
// Records port, pin, state. Tests check bsw_stub_last_write_state.
inline GPIO_PinState bsw_stub_last_write_state = GPIO_PIN_RESET;


inline void bsw_stub_hal_gpio_init(GPIO_TypeDef* port, const GPIO_InitTypeDef* init) {
    bsw_stub_last_port = port;
    if (init) bsw_stub_last_pin = init->Pin;
}

inline void bsw_stub_hal_gpio_deinit(GPIO_TypeDef* port, uint16_t pin) {
    bsw_stub_last_port = port;
    bsw_stub_last_pin = pin;
}

inline void bsw_stub_hal_gpio_write(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state) {
    bsw_stub_last_port = port;
    bsw_stub_last_pin = pin;
    bsw_stub_last_write_state = state;
}

// Returns GPIO_PIN_RESET for reads by default.
inline GPIO_PinState bsw_stub_hal_gpio_read(GPIO_TypeDef* port, uint16_t pin) {
    bsw_stub_last_port = port;
    bsw_stub_last_pin = pin;
    return GPIO_PIN_RESET;
}

inline void bsw_stub_hal_gpio_toggle(GPIO_TypeDef* port, uint16_t pin) {
    bsw_stub_last_port = port;
    bsw_stub_last_pin = pin;
}

// Stub macros for compiling entirely testably with the flag
#define BSW_HAL_GPIO_INIT(port, initStruct)     bsw_stub_hal_gpio_init(port, &(initStruct))
#define BSW_HAL_GPIO_DEINIT(port, pin)          bsw_stub_hal_gpio_deinit(port, pin)
#define BSW_HAL_GPIO_WRITE(port, pin, state)    bsw_stub_hal_gpio_write(port, pin, state)
#define BSW_HAL_GPIO_READ(port, pin)            bsw_stub_hal_gpio_read(port, pin)
#define BSW_HAL_GPIO_TOGGLE(port, pin)          bsw_stub_hal_gpio_toggle(port, pin)
