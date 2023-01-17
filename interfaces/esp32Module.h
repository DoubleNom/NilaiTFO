/**
 * @addtogroup esp32Module.h
 * @{
 *******************************************************************************
 * @file	esp32Module.h
 * @author	Samuel Martel
 * @brief
 * Created on: Apr 9, 2021
 *******************************************************************************
 */

#ifndef ESP32MODULE_H_
#define ESP32MODULE_H_

/***********************************************/
/* Includes */
#if defined(NILAI_USE_ESP32)
#if !defined(NILAI_USE_UART)
#error The UART module must be enabled to use the ESP32 Module
#endif

#include "defines/internalConfig.h"
#include NILAI_HAL_HEADER
#include "defines/module.hpp"
#include "defines/pin.h"

#include <array>
#include <drivers/uart/module.hpp>
#include <map>
#include <string_view>
#include <utility>

namespace Nilai::Interfaces::Esp32 {
/**
 * Boot modes for the ESP32.
 */
    enum class BootMode {
        Bootloader = 0,    //!< Bootloader
        Normal = 1,    //!< Normal
    };

/**
 * Firmware files for flashing ESP32
 */
    using namespace std::literals;

    struct Firmware {
        struct File {
            uint32_t address;
            std::string name;
            std::string path;

            File(std::string name, uint32_t address) : address(address), name(std::move(name)) {}
        };

        std::string folder;
        std::vector<File> files;

        Firmware() = default;

        Firmware(std::string folder, std::vector<File> files) : folder(std::move(folder)), files(std::move(files)) {
            for (auto &file: this->files) {
                file.path = this->folder + "/" + file.name;
            }
        }

        [[nodiscard]] std::string GetNoEraseFilePath() const { return folder + "/debug"; }
    };

/**
 * Contains all the control pins of the ESP32 module.
 */
    struct Pins {
        Nilai::Defines::Pin enable = {};    //!< Enable pin of the ESP32. When high, the ESP32 is enabled.
        Nilai::Defines::Pin boot = {};    /**< Boot selection pin.
                                         *   When high, normal boot.
                                         *   When low, runs the bootloader.
                                         */
        Nilai::Defines::Pin tpout = {};     //!< Heartbeat pin from the ESP32 to the STM32.
        Nilai::Defines::Pin tpin = {};     //!< Debug signal from the STM32 to the ESP32, currently not used.
    };

    class Module : public Nilai::Drivers::Uart::Module {
    public:
        Module(const std::string &label, UART_HandleTypeDef *uart, const Pins &pins, std::string version);

        ~Module() override = default;

        bool DoPost() override;

        void Run() override;

        /**
         * Enables the ESP32 through the enable pin.
         * Will reset the ESP during process
         * @return true on successfull boot
         */
        bool Enable(BootMode mode = BootMode::Normal);

        /**
         * Disables the ESP32 through the enable pin.
         */
        void Disable();

        /**
         * Checks if the ESP32 is currently enabled.
         * @return True if the ESP32 is enabled, false otherwise.
         */
        [[nodiscard]] bool IsEnabled() const { return m_pins.enable.Get(); }

        /**
         * Set Firmware used for flashing ESP on POST
         * Default firmware (empty) will disable flashing procedure.
         * @param firmware structure of the binary files
         */
        [[maybe_unused]] void SetFirmware(const Firmware &firmware) { m_firmware = std::move(firmware); }

        /**
         * Set the user data to send to the ESP if required
         * @param data byte array to send to the ESP
         * @param len length of the array
         */
        [[maybe_unused]] void SetUserData(uint8_t *data, size_t len) {
            m_userData.insert(m_userData.end(), data, data + len);
        }

        [[maybe_unused]] void SetUserData(const std::vector<uint8_t> &data) { m_userData = data; }

        [[maybe_unused]] void
        SetFlashProgressCallback(const std::function<void(size_t progress, size_t size)> &callback) {
            m_flashProgressCallback = callback;
        }

        [[maybe_unused]] void SetFlashDoneCallback(const std::function<void()> &callback) {
            m_flashDone = callback;
        }

    private:
        void SendUserData();

        bool Bootloader();

        bool PrepareFlash();

        bool
        FlashBinary(size_t address, size_t file_size, size_t block_size, const std::function<size_t(uint8_t *)> &cb);

    private:
        Pins m_pins = {};
        std::string m_version;
        Firmware m_firmware;
        std::vector<uint8_t> m_userData;
        std::function<void(size_t progress, size_t size)> m_flashProgressCallback = [](size_t, size_t) {};
        std::function<void()> m_flashDone = []() {};

    private:
        static constexpr size_t TIMEOUT = 5 * 1000;
    };
}    // namespace Nilai::Interfaces::Esp32
/**
 * @}
 */
/* END OF FILE */
#endif
#endif /* ESP32MODULE_H_ */
