# Architectural Decisions

This file documents every architectural decision made before writing any code.
It exists so that anyone reading the repo — including the author six months later —
understands why things are the way they are, not just what they are.

## 1. Language: C++17
- **Decision:** C++17
- **Why:** Modern enough to use enum class, structured bindings, and if constexpr. Broad toolchain support on ARM GCC. No features that cause problems on embedded targets.
- **What was rejected:** C++14 (too conservative), C++20 (toolchain support patchy on embedded) And no experience with the others.

## 2. Class per peripheral
- **Decision:** One class per peripheral driver.
- **Why:** Groups state and behaviour together. Constructor handles init. Natural fit for hardware that has a fixed lifecycle — init, use, deinit.
- **What was rejected:** Namespace + free functions. Would require passing state around explicitly. No real advantage for single-instance hardware peripherals.

## 3. Singleton instance
- **Decision:** One instance per hardware peripheral. No enforcement mechanism — by convention.
- **Why:** Hardware is a singleton by nature. One UART2, one SPI1. Enforcing it in code adds complexity with no practical benefit on a bare-metal target.
- **What was rejected:** Injectable instances. Better for testing in theory, but adds indirection that embedded developers do not need. Straightforward solutions preferred.

## 4. Constructor initialises hardware
- **Decision:** Constructor calls HAL init. No separate init() method.
- **Why:** An object that exists should be usable. A two-step init is a common source of bugs — forgetting to call init() after construction.
- **Tradeoff:** Constructors cannot return a status code. Handled with an _initialized flag.
- **What was rejected:** Static factory method (Gpio::create()). Cleaner failure handling but adds indirection.

## 5. _initialized flag
- **Decision:** Constructor sets _initialized = true on success. Every method checks it.
- **Why:** Simple. No exceptions. No undefined behaviour if init failed. Caller checks isReady() if they need to verify.
- **What was rejected:** Exceptions. Better to check for errors than to throw exceptions.

## 6. BswResult<T> return type
- **Decision:** template<typename T> struct BswResult { Status status; T value; };
- **Why:** Functions that return a value need to return a status too. A single generic type avoids writing GpioReadResult, UartReadResult, AdcReadResult separately. Caller uses C++17 structured bindings: auto [status, value] = driver.read();
- **What was rejected:** Out parameters. Work fine but less readable. Old style.

## 7. Status error codes
- **Decision:** enum class Status : uint8_t. Upper nibble = peripheral family. Lower nibble = error.
- **Why:** Single type for all drivers. Upper nibble makes log parsing easy. 0x1X is always UART, 0x2X is always SPI, etc. enum class prevents implicit conversion to int.
- **What was rejected:** Per-peripheral error enums. Would require casting or overloading at every call site. Wanted to keep it simple.

## 8. HAL adapter layer
- **Decision:** gpio_adapter.hpp is the only file that includes STM32 HAL headers. All HAL calls go through macros defined in the adapter.
- **Why:** Isolates STM32 dependency to one file. Porting to a different target means replacing one file, not hunting through driver code. Also makes host-side unit testing possible — swap the adapter for a stub.
- **What was rejected:** Calling HAL directly in gpio.cpp. Simpler but couples the driver to STM32 forever.

## 9. enum class for all enums
- **Decision:** All enums use enum class.
- **Why:** No implicit conversion to int. No name collisions with HAL constants. Forces explicit type at every use site. Catches mistakes at compile time.
- **What was rejected:** Plain enum. Works but pollutes the enclosing scope and allows silent integer conversion.

## 10. Host-side unit tests with HAL stub
- **Decision:** Tests compile on host with plain g++. gpio_adapter.hpp replaced by gpio_hal_stub.hpp at compile time using -DBSW_HAL_STUB.
- **Why:** No hardware or cross-compiler needed to run tests. Any machine can verify driver logic. Fast feedback loop.
- **Tradeoff:** Stub tests verify logic only, not timing, not real hardware behaviour. Real hardware validation still required.
- **What was rejected:** QEMU. Solves functional simulation but not timing. Adds toolchain complexity for logic tests that a simple stub handles better.

## 11. TX ring buffer in the UART driver
- **Decision:** Uart owns a fixed-size TX ring buffer (`_txBuf[BSW_UART_TX_BUF_SIZE]`). `send_IT()` copies caller data in; `irqHandler()` drains it one byte at a time via HAL IT.
- **Why:** HAL IT transmit can only be armed once per transfer. Without an internal buffer, the caller would have to keep their buffer alive until TX completes — an invisible contract that causes hard-to-find bugs.
- **What was rejected:** Caller-managed TX buffer (same lifetime requirement as above). DMA (adds descriptor complexity; ring buffer is sufficient for typical UART rates).

## 12. Caller-supplied RX buffer
- **Decision:** `UartConfig::rxBuf` and `rxBufSize` are provided by the caller. The driver does not allocate memory.
- **Why:** Embedded targets have constrained RAM. The caller knows their message size; the driver does not. Caller-supplied buffers also avoid hidden heap use on bare-metal targets where `new` may be disabled.
- **What was rejected:** Driver-owned RX buffer. Would require a compile-time size or dynamic allocation — both impose policy the driver should not own.

## 13. Callbacks run in interrupt context
- **Decision:** `onRxData` and `onTxComplete` are called directly from `irqHandler()`, which itself is called from the HAL UART IRQ callback.
- **Why:** Zero-copy, zero-latency notification. The callback is the fastest way to get data to the application layer without introducing a second buffer or RTOS queue inside the driver.
- **Tradeoff:** Callbacks must be short and non-blocking. Documented in `UartConfig` comments. Violating this stalls the interrupt and corrupts timing.
- **What was rejected:** Internal queue + polling. Adds latency and RAM overhead. The right layer for that is the application, not the BSW driver.

## 14. UART adapter follows the same HAL isolation pattern as GPIO
- **Decision:** `uart_adapter.hpp` is the only file that includes STM32 UART HAL headers. All HAL calls go through macros. `uart_hal_stub.hpp` replaces the macros at compile time when `BSW_HAL_STUB` is defined.
- **Why:** Consistent pattern across all drivers. Anyone who understands the GPIO adapter immediately understands the UART adapter. Portability and testability work the same way.
- **What was rejected:** Calling HAL directly in uart.cpp. Same reason as decision 8 — couples the driver to STM32 forever.

## 15. BSW_UART_TX_BUF_SIZE in bsw_config.hpp
- **Decision:** TX buffer size is a compile-time `#define` in `bsw_config.hpp`, not a template parameter or runtime value.
- **Why:** The buffer is a stack-allocated array inside `Uart`. Its size must be known at compile time. A `#define` in one central file is the simplest way to tune it per project without touching driver source.
- **What was rejected:** Template parameter (`Uart<128>`). Works but adds template noise at every instantiation site. `#define` is sufficient for a single tuneable constant.
