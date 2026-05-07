// UART HAL stub for host-side testing. Replaces uart_adapter.hpp at compile time
// with -DBSW_HAL_STUB. Records calls so tests can assert driver behaviour without hardware.
#pragma once

#include <cstdint>

struct UART_InitTypeDef {
    uint32_t BaudRate;
    uint32_t WordLength;
    uint32_t StopBits;
    uint32_t Parity;
    uint32_t Mode;
    uint32_t HwFlowCtl;
};

// Init member must be UART_InitTypeDef so uart.cpp can assign buildHalUartInit() into it.
struct UART_HandleTypeDef {
    UART_InitTypeDef Init;
};

enum HAL_StatusTypeDef { HAL_OK = 0, HAL_ERROR = 1, HAL_BUSY = 2, HAL_TIMEOUT = 3 };

// HAL constants used by uart_adapter.hpp translation function.
#define UART_WORDLENGTH_8B   0x00u
#define UART_WORDLENGTH_9B   0x01u
#define UART_STOPBITS_1      0x00u
#define UART_STOPBITS_2      0x02u
#define UART_PARITY_NONE     0x00u
#define UART_PARITY_EVEN     0x02u
#define UART_PARITY_ODD      0x04u
#define UART_MODE_TX_RX      0x03u
#define UART_HWCONTROL_NONE  0x00u

// Tests set these to inject return values and read them to verify calls.
inline bool              bsw_stub_uart_tx_called    = false;
inline bool              bsw_stub_uart_rx_called    = false;
inline bool              bsw_stub_uart_tx_it_called = false;
inline uint16_t          bsw_stub_uart_last_len     = 0;
inline HAL_StatusTypeDef bsw_stub_uart_hal_return   = HAL_OK;

// Records tx call and last length. (void) casts suppress unused-parameter warnings.
#define BSW_HAL_UART_TX(handle, buf, len, timeout) \
    ((void)(handle), (void)(buf), (void)(timeout), \
     bsw_stub_uart_tx_called = true, \
     bsw_stub_uart_last_len = static_cast<uint16_t>(len), \
     bsw_stub_uart_hal_return)

// Records rx call. (void) casts suppress unused-parameter warnings.
#define BSW_HAL_UART_RX(handle, buf, len, timeout) \
    ((void)(handle), (void)(buf), (void)(timeout), \
     bsw_stub_uart_rx_called = true, \
     bsw_stub_uart_last_len = static_cast<uint16_t>(len), \
     bsw_stub_uart_hal_return)

// Records tx_it call.
#define BSW_HAL_UART_TX_IT(handle, buf, len) \
    ((void)(handle), (void)(buf), \
     bsw_stub_uart_tx_it_called = true, \
     bsw_stub_uart_last_len = static_cast<uint16_t>(len), \
     HAL_OK)

// (void) cast silences "statement has no effect" when used as a standalone call.
#define BSW_HAL_UART_RX_IT(handle, buf, len) \
    ((void)(handle), (void)(buf), (void)(len))

#define BSW_HAL_UART_DEINIT(handle) \
    ((void)(handle))

inline HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef*) { return HAL_OK; }
