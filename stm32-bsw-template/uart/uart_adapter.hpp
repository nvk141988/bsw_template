// Only file that includes STM32 HAL UART headers.
// Replace this file to port to a different target.
#pragma once

#ifndef BSW_HAL_STUB
#include "stm32xxxx_hal.h"  // placeholder — update to your STM32 family

// Maps to HAL_UART_Transmit. Blocking TX.
#define BSW_HAL_UART_TX(handle, buf, len, timeout)  HAL_UART_Transmit(handle, buf, len, timeout)

// Maps to HAL_UART_Receive. Blocking RX. HAL enforces the timeout.
#define BSW_HAL_UART_RX(handle, buf, len, timeout)  HAL_UART_Receive(handle, buf, len, timeout)

// Maps to HAL_UART_Transmit_IT. Non-blocking TX.
#define BSW_HAL_UART_TX_IT(handle, buf, len)        HAL_UART_Transmit_IT(handle, buf, len)

// Maps to HAL_UART_Receive_IT. Non-blocking RX.
#define BSW_HAL_UART_RX_IT(handle, buf, len)        HAL_UART_Receive_IT(handle, buf, len)

// Maps to HAL_UART_DeInit.
#define BSW_HAL_UART_DEINIT(handle)                 HAL_UART_DeInit(handle)
#endif // BSW_HAL_STUB

namespace Bsw {

// Translates BSW config enums to STM32 HAL constants.
// Lives here because only the adapter should know HAL types.
inline UART_InitTypeDef buildHalUartInit(const UartConfig& cfg) {
    UART_InitTypeDef init{};
    init.BaudRate = cfg.baudRate;

    // UartWordLength → STM32 word length constant
    switch (cfg.wordLength) {
        case UartWordLength::Bits8: init.WordLength = UART_WORDLENGTH_8B; break;
        case UartWordLength::Bits9: init.WordLength = UART_WORDLENGTH_9B; break;
    }

    // UartStopBits → STM32 stop bits constant
    switch (cfg.stopBits) {
        case UartStopBits::One: init.StopBits = UART_STOPBITS_1; break;
        case UartStopBits::Two: init.StopBits = UART_STOPBITS_2; break;
    }

    // UartParity → STM32 parity constant
    switch (cfg.parity) {
        case UartParity::None: init.Parity = UART_PARITY_NONE; break;
        case UartParity::Even: init.Parity = UART_PARITY_EVEN; break;
        case UartParity::Odd:  init.Parity = UART_PARITY_ODD;  break;
    }

    init.Mode      = UART_MODE_TX_RX;
    init.HwFlowCtl = UART_HWCONTROL_NONE;

    return init;
}

} // namespace Bsw
