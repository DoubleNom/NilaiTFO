/**
 * @addtogroup  drivers
 * @{
 * @addtogroup  uart
 * @{
 * @file        uartModule.hpp
 * @author      Samuel Martel
 * @author      Pascal-Emmanuel Lachance
 * @date        2020/08/13  -  09:25
 *
 * @brief       UART communication module
 */
#ifndef GUARD_UARTMODULE_HPP
#define GUARD_UARTMODULE_HPP
/*************************************************************************************************/
/* Includes
 * ------------------------------------------------------------------------------------
 */
#if defined(NILAI_USE_UART)

#include "defines/internalConfig.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include NILAI_HAL_HEADER

#pragma GCC diagnostic pop
#if defined(HAL_UART_MODULE_ENABLED)

#include "defines/circular_buffer.hpp"
#include "defines/macros.hpp"
#include "defines/misc.hpp"
#include "defines/module.hpp"
#include "frame.h"
#include "status.h"

#include <cstdint>       // For uint8_t, size_t
#include <functional>    // For std::function
#include <string>        // For std::string
#include <vector>        // For std::vector

#ifdef GTEST
#include <gtest/gtest_prod.h>
#endif

namespace Nilai::Drivers::Uart {

enum class SectionState {
    NotComplete,
    Complete,
};

class Module : public cep::Module {
  public:
    Module(const std::string& label, UART_HandleTypeDef* uart, size_t txl = 512, size_t rxl = 512);

    ~Module();

    bool DoPost();

    void Run();

    [[nodiscard]] const std::string& GetLabel() const override {
        return m_label;
    }

    /**
     * Non blocking send for bytes buffer
     * @param msg bytes to send
     * @param len number of bytes to send
     */
    void Transmit(const char* msg, size_t len);

    /**
     * Blocking send
     * does not cancel transmission on timeout
     * @param msg bytes to send
     * @param len number of bytes to send
     * @param timeout waiting time for transmission
     * @return false on timeout
     */
    bool Transmit(const char* msg, size_t len, uint32_t timeout);

    void Transmit(const uint8_t* buff, size_t len);

    bool Transmit(const uint8_t* buff, size_t len, uint32_t timeout);

    void Transmit(const std::string& msg);

    void Transmit(const std::vector<uint8_t>& msg);

    [[maybe_unused]] void VTransmit(const char* fmt, ...);

    size_t AvailableBytes() {
        return m_rxCirc.size();
    }

    size_t AvailableFrames() {
        return m_rxFrames.size();
    }

    size_t Receive(uint8_t* buf, uint8_t len);

    size_t Receive(uint8_t* buf, uint8_t len, uint32_t timeout);

    Frame Receive();

    void SetExpectedRxLen(size_t len);

    void ClearExpectedRxLen();

    void SetFrameReceiveCpltCallback(const std::function<void()>& cb);

    void ClearFrameReceiveCpltCallback();

    void SetStartOfFrameSequence(const std::string& sof);

    void SetStartOfFrameSequence(const std::vector<uint8_t>& sof);

    void SetStartOfFrameSequence(uint8_t* sof, size_t len);

    void ClearStartOfFrameSequence();

    void SetEndOfFrameSequence(const std::string& eof);

    void SetEndOfFrameSequence(const std::vector<uint8_t>& eof);

    void SetEndOfFrameSequence(uint8_t* eof, size_t len);

    void ClearEndOfFrameSequence();

    void FlushRecv();

  private:
    bool WaitUntilTransmissionComplete(uint32_t timeout = TX_TIMEOUT);

    bool ResizeDmaBuffer();

    void SetTriage();

    void SearchFrame(
      std::vector<uint8_t> data,
      std::vector<uint8_t> pattern,
      std::vector<size_t>& result,
      size_t               max_depth = 0,
      size_t               offset    = 0);

  protected:
    std::string          m_label;

  private:
    UART_HandleTypeDef* m_handle = nullptr;

    Status m_status = Status::Ok;

    std::string m_sof;        // start of frame
    std::string m_eof;        // end of frame
    size_t      m_efl = 0;    // expected frame length

    size_t m_txl;    // transmission buffer size
    size_t m_rxl;    // reception buffer size

    size_t                  m_sBuffId = 0;    // Static buffer id
    std::vector<uint8_t>    m_txBuff;         // transmission buffer
    std::vector<uint8_t>    m_rxBuff;    // reception buffer
    CircularBuffer<uint8_t> m_rxCirc;         // circular read access for reception buffer
    CircularBuffer<Frame>   m_rxFrames;       // reception frame buffer

    std::function<void()> m_cb;

    // Triage
    std::function<void()> m_triage;

    static constexpr uint32_t TX_TIMEOUT = 100;    // Systicks.
    static constexpr uint32_t RX_TIMEOUT = 50;     // Systicks.


    private:
    #ifdef GTEST
      FRIEND_TEST(Uart, TriageSof);
      FRIEND_TEST(Uart, TriageEof);
      FRIEND_TEST(Uart, TriageSofEof);
      FRIEND_TEST(Uart, TriageElf);
      FRIEND_TEST(Uart, TriageNone);
    #endif
};
}    // namespace Nilai::Drivers::Uart

#else
#if WARN_MISSING_STM_DRIVERS
#warning NilaiTFO UART module enabled, but HAL_UART_MODULE_ENABLE is not defined!
#endif
#endif
#endif
#endif

/**
 * @}
 * @}
 */
/* ----- END OF FILE ----- */
