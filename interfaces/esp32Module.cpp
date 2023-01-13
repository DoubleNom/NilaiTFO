/**
 * @addtogroup esp32Module.cpp
 * @{
 *******************************************************************************
 * @file	esp32Module.cpp
 * @author	Samuel Martel
 * @brief
 * Created on: Apr 9, 2021
 *******************************************************************************
 */

#include "esp32Module.h"

#include <utility>

#if defined(NILAI_USE_ESP32)
#if !defined(NILAI_USE_UART)
#error The UART module must be enabled in order to use the ESP32 Module
#endif

#include "defines/internalConfig.h"
#include NILAI_HAL_HEADER
#include "libs/esp-serial-flasher/include/esp_loader.h"
#include "libs/esp-serial-flasher/port/nilai_port.h"
#include "services/file.h"
#include "services/filesystem.h"

#define ESP_DEBUG(msg, ...) LOGTD(m_label.c_str(), msg __VA_OPT__(, ) __VA_ARGS__)
#define ESP_INFO(msg, ...)  LOGTI(m_label.c_str(), msg __VA_OPT__(, ) __VA_ARGS__)
#define ESP_WARN(msg, ...)  LOGTW(m_label.c_str(), msg __VA_OPT__(, ) __VA_ARGS__)
#define ESP_ERROR(msg, ...) LOGTE(m_label.c_str(), msg __VA_OPT__(, ) __VA_ARGS__)

namespace Nilai::Interfaces::Esp32 {

    Module::Module(const std::string &label, UART_HandleTypeDef *uart, const Pins &pins, std::string version)
            : Nilai::Drivers::Uart::Module(label, uart, 4500, 4500), m_pins(pins), m_version(std::move(version)) {
        ESP_INFO("ESP Initialized");
    }

/**
 * To pass the POST, the following things must all succeed:
 *  - The `tpout` must be HIGH
 *  - The ESP must respond with "OK".
 * @return True if the POST passes, false otherwise.
 */
    bool Module::DoPost() {
        if (!Bootloader()) {
            ESP_ERROR("POST: bootloader failure");
            return false;
        }

        if (!Enable(BootMode::Normal)) {
            ESP_ERROR("POST: Failed to start ESP");
            return false;
        }

        // Make sure that TPOUT is high.
        if (!m_pins.tpout.Get()) {
            ESP_ERROR("POST: TPOUT is LOW!");
            return false;
        }

        // POST OK!
        ESP_INFO("POST OK");

        return true;
    }

    void Module::Run() {
        // If TPOUT is LOW, there's a problem, reset the ESP.
        if (!m_pins.tpout.Get()) {
            ESP_WARN("TPOUT is LOW, resetting!");
            Enable(BootMode::Normal);
        }
        Nilai::Drivers::Uart::Module::Run();
    }

    bool Module::Enable(BootMode mode) {
        m_pins.enable.Set(false);
        m_pins.boot.Set(false);
        m_pins.tpin.Set(false);
        HAL_Delay(1);

        if (mode == BootMode::Bootloader) {
            // io2 must be set to false when loading as bootloader
            // Otherwise, it will power-cycle
            m_pins.tpin.Set(false);
            m_pins.boot.Set(false);
        } else {
            m_pins.tpin.Set(true);
            m_pins.boot.Set(true);
        }

        m_pins.enable.Set(true);

        if (mode == BootMode::Normal) {
            SetStartOfFrameSequence("\x01\x02");
            SetEndOfFrameSequence("\x03\x04");
            SetEscapeSequence("\033");


            // Wait for ESP to enable TPOUT
            size_t deadline = HAL_GetTick() + Module::TIMEOUT;
            while (!m_pins.tpout.Get()) {
                if (HAL_GetTick() >= deadline) {
                    ESP_ERROR("TPOUT unchanged");
                    return false;
                }
            }

            SendUserData();

            // Wait for its response.
            deadline = HAL_GetTick() + Module::TIMEOUT;
            while (AvailableFrames() == 0) {
                Nilai::Drivers::Uart::Module::Run();
                if (HAL_GetTick() >= deadline) {
                    // Timed out.
                    ESP_ERROR("No response from ESP!");
                    return false;
                }
            }
            Nilai::Drivers::Uart::Frame frame = Receive();
            std::string reply = std::string(frame.data.begin(), frame.data.end());
            if (reply != "OK") {
                ESP_ERROR("Invalid response from ESP! %s", reply.c_str());
                return false;
            }
        }
        return true;
    }

    void Module::Disable() {
        m_pins.enable.Set(false);
        m_pins.boot.Set(false);
        m_pins.tpin.Set(false);
    }

    void Module::SendUserData() {
        if (!m_userData.empty()) Transmit((const char *) m_userData.data(), m_userData.size());    // Send the user data
    }

    bool Module::Bootloader() {
        // No firmware provided, skip procedure
        if (m_firmware.files.empty()) return true;

        cep::Filesystem::fileInfo_t fileInfo;
        cep::Filesystem::Result err;


        // Check if user put firmware folder
        err = cep::Filesystem::GetStat(m_firmware.folder, &fileInfo);
        if (err != cep::Filesystem::Result::Ok) {
            return true;
        }

        // Check if all files required are provided
        bool hasAllFiles = true;
        for (const auto &file: m_firmware.files) {
            err = cep::Filesystem::GetStat(file.path, &fileInfo);
            if (err != cep::Filesystem::Result::Ok) {
                ESP_ERROR("%s is missing", file.name.c_str());
                hasAllFiles = false;
            }
        }
        if (!hasAllFiles) {
            return false;
        }

        // Prepare esp_tool
        loader_port_nilai_init(this, m_pins.enable, m_pins.boot);

        // Connect to ESP
        if (!PrepareFlash()) {
            return false;
        }

        // Flash ESP
        for (const auto &file: m_firmware.files) {
            cep::Filesystem::File f(file.path, cep::Filesystem::FileModes::Read);

            if (!f.IsOpen()) {
                ESP_ERROR("Failed to open %s", file.name.c_str());
                return false;
            }

            static constexpr size_t blockSize = 1024;
            if (!FlashBinary(file.address, f.GetSize(), blockSize, [&](uint8_t *payload) {
                size_t lenRead = 0;
                err = f.Read(payload, blockSize, &lenRead);
                if (err != cep::Filesystem::Result::Ok) {
                    lenRead = 0;
                }
                return lenRead;
            })) {
                f.Close();
                return false;
            }
            f.Close();
            ESP_INFO("Flashed %s", file.name.c_str());
        }

        err = cep::Filesystem::GetStat(m_firmware.GetNoEraseFilePath(), &fileInfo);
        if (err != cep::Filesystem::Result::Ok) {
            ESP_DEBUG("Erasing firmware folder");
            for (const auto &file: m_firmware.files) {
                err = cep::Filesystem::Unlink(file.path);
                if (err != cep::Filesystem::Result::Ok) {
                    ESP_ERROR("Failed to delete folder");
                    return false;
                }
            }
            err = cep::Filesystem::Unlink(m_firmware.folder);
            if (err != cep::Filesystem::Result::Ok) {
                ESP_ERROR("Failed to delete folder");
                return false;
            }
        }

        return true;
    }

    bool Module::PrepareFlash() {
        // Disable ESP and reset its pins to default state
        Disable();

        // Connect to target
        esp_loader_connect_args_t connect_config = ESP_LOADER_CONNECT_DEFAULT();
        esp_loader_error_t err = esp_loader_connect(&connect_config);
        if (err != ESP_LOADER_SUCCESS) {
            ESP_ERROR("Cannot connect to target, Error: %u", err);
            return false;
        }
        ESP_DEBUG("Connected to target");
        return true;
    }

    bool Module::FlashBinary(
            size_t address,
            size_t file_size,
            size_t block_size,
            const std::function<size_t(uint8_t *payload)> &cb) {
        esp_loader_error_t err;
        ESP_DEBUG("Erasing flash");
        err = esp_loader_flash_start(address, file_size, block_size);
        if (err != ESP_LOADER_SUCCESS) {
            ESP_ERROR("Erasing flash failed with error %d", err);
            return false;
        }
        ESP_DEBUG("Start programming");

        std::vector<uint8_t> payload = std::vector<uint8_t>(block_size);
        size_t last_block_size;
        size_t written = 0;
        do {
            last_block_size = cb(payload.data());
            if (last_block_size == 0) {
                ESP_ERROR("Failure when loading binary from source");
                return false;
            }
            err = esp_loader_flash_write(payload.data(), last_block_size);
            if (err != ESP_LOADER_SUCCESS) {
                ESP_ERROR("Packet could not be written! error %d", err);
                return false;
            }
            Logger::Log("Flashing %02d%% [%d/%d]\r", written * 100 / file_size, written, file_size);
            m_flashProgressCallback(written, file_size);
            written += last_block_size;
        } while (written < file_size);
        m_flashProgressCallback(file_size, file_size);
        ESP_DEBUG("Finished programming");

        err = esp_loader_flash_verify();
        if (err != ESP_LOADER_SUCCESS) {
            ESP_ERROR("MD5 does not match. err: %d", err);
            return false;
        }
        ESP_DEBUG("Flash verified");
        return true;
    }
}    // namespace Nilai::Interfaces::Esp32

#endif
