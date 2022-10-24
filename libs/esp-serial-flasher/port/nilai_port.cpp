#if defined(NILAI_USE_ESP32)
#if !defined(NILAI_USE_UART)
#error The UART module must be enabled in order to use the ESP32 Module
#endif

#include "nilai_port.h"

#include "drivers/uart/module.hpp"

#include <stdint.h>
#include <stdio.h>
#include <sys/param.h>

// #define SERIAL_DEBUG_ENABLE

static Nilai::Drivers::Uart::Module* uart;
static Nilai::Defines::Pin           en, boot;

#ifdef SERIAL_DEBUG_ENABLE

static void serial_debug_print(const uint8_t* data, uint16_t size, bool write) {
    static bool write_prev = false;

    if (write_prev != write) {
        write_prev = write;
        Logger::Log("\r\n--- %s ---\r\n", write ? "WRITE" : "READ");
    }

    for (uint32_t i = 0; i < size; i++) {
        Logger::Log("%02x ", data[i]);
    }
}

#else

static void serial_debug_print(const uint8_t* data, uint16_t size, bool write) {
    UNUSED(data);
    UNUSED(size);
    UNUSED(write);
}

#endif

extern "C" {

    static uint32_t s_time_end;

    esp_loader_error_t loader_port_serial_write(const uint8_t* data, uint16_t size, uint32_t timeout) {
        serial_debug_print(data, size, true);

        return uart->Transmit(data, size, timeout) ? ESP_LOADER_SUCCESS : ESP_LOADER_ERROR_TIMEOUT;
    }

    esp_loader_error_t loader_port_serial_read(uint8_t* data, uint16_t size, uint32_t timeout) {
        size_t len = uart->Receive(data, static_cast<uint8_t>(size), timeout);

        serial_debug_print(data, size, false);

        return len == size ? ESP_LOADER_SUCCESS : ESP_LOADER_ERROR_TIMEOUT;
    }

    void loader_port_stm32_reinit_uart() { }

    // Set GPIO0 LOW, then
    // assert reset pin for 100 milliseconds.
    void loader_port_enter_bootloader(void) {
        en.Set(false);
        boot.Set(false);
        HAL_Delay(1);
        en.Set(true);
        HAL_Delay(100);
        boot.Set(true);

        HAL_Delay(450);
        uart->FlushRecv();
    }

    void loader_port_reset_target(void) {
        en.Set(false);
        HAL_Delay(100);
        en.Set(true);
    }

    void loader_port_delay_ms(uint32_t ms) { HAL_Delay(ms); }

    void loader_port_start_timer(uint32_t ms) { s_time_end = HAL_GetTick() + ms; }

    uint32_t loader_port_remaining_time(void) {
        int32_t remaining = s_time_end - HAL_GetTick();
        return (remaining > 0) ? (uint32_t)remaining : 0;
    }

    void loader_port_debug_print(const char* str) { LOGTD("FESP", "%s", str); }

    [[maybe_unused]] esp_loader_error_t loader_port_change_baudrate(uint32_t baudrate) {
        UNUSED(baudrate);
        return ESP_LOADER_ERROR_FAIL;
    }
}

void loader_port_nilai_init(Nilai::Drivers::Uart::Module* _uart, Nilai::Defines::Pin _en, Nilai::Defines::Pin _boot) {
    uart = _uart;
    en   = _en;
    boot = _boot;
    uart->ClearStartOfFrameSequence();
    uart->ClearEndOfFrameSequence();
    uart->ClearExpectedRxLen();
}

#endif
