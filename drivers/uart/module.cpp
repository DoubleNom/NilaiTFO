/**
 * @addtogroup  drivers
 * @{
 * @addtogroup  uart
 * @{
 * @file        uartModule.cpp
 * @author      Samuel Martel
 * @author      Pascal-Emmanuel Lachance
 * @date        2020/08/13  -  09:40
 *
 * @brief       UART communication module
 */
/*************************************************************************************************/
/* Includes
 * ------------------------------------------------------------------------------------
 */
#include "module.hpp"

#if defined(NILAI_USE_UART) && defined(HAL_UART_MODULE_ENABLED)

#include "interval.h"
#include "main.h"

#include <algorithm>
#include <cstdarg>    // For va_list.
#include <cstdio>
#include <cstring>
#include <iterator>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace Nilai::Drivers::Uart {

Module::Module(const std::string& label, UART_HandleTypeDef* uart, size_t txl, size_t rxl)
: m_label(label)
, m_handle(uart)
, m_txl(txl)
, m_rxl(rxl)
, m_rxFrames(10) {
    CEP_ASSERT(uart != nullptr, "UART Handle is NULL!");
    m_txBuff.resize(txl);
    m_rxBuff.resize(rxl);

    m_rxCirc.init(m_rxBuff.data(), rxl);

    m_sof.reserve(2);
    m_eof.reserve(2);

    m_triage = []() {
    };
    m_cb = [](){
    };
    HAL_UART_Receive_DMA(m_handle, m_rxBuff.data(), rxl);
    //    __HAL_UART_ENABLE_IT(m_handle, UART_IT_RXNE);
    LOGTI(label.c_str(), "Uart initialized");
}

Module::~Module() {
    HAL_UART_DeInit(m_handle);
}

/**
 * If the initialization passed, the POST passes.
 * @return
 */
bool Module::DoPost() {
    LOGTI(m_label.c_str(), "POST OK!");
    return true;
}

void Module::Run() {
    size_t len = m_rxCirc.dmaCounter(__HAL_DMA_GET_COUNTER(m_handle->hdmarx));
    if (len != 0) {
        m_triage();
    }
}

void Module::Transmit(const char* msg, size_t len) {
    CEP_ASSERT(msg != nullptr, "msg is NULL in UartModule::Transmit");

    if (!WaitUntilTransmissionComplete()) {
        // Timed out.
        return;
    }

    // Copy the message into the transmission buffer.
    m_txBuff.resize(len);
    memcpy((void*)m_txBuff.data(), (void*)msg, len);

    // Send the message.
    if (HAL_UART_Transmit_IT(m_handle, m_txBuff.data(), (uint16_t)m_txBuff.size()) != HAL_OK) {
        LOGTE(m_label.c_str(), "In Transmit: Unable to transmit message");
        return;
    }
}

bool Module::Transmit(const char* msg, size_t len, uint32_t timeout) {
    Transmit(msg, len);
    return WaitUntilTransmissionComplete(timeout);
}

void Module::Transmit(const uint8_t* buff, size_t len) {
    CEP_ASSERT(buff != nullptr, "msg is NULL in UartModule::Transmit");

    if (!WaitUntilTransmissionComplete()) {
        // Timed out.
        return;
    }

    // Copy the message into the transmission buffer.
    m_txBuff.resize(len);
    memcpy(m_txBuff.data(), buff, len);

    // Send the message.
    if (HAL_UART_Transmit_IT(m_handle, m_txBuff.data(), (uint16_t)m_txBuff.size()) != HAL_OK) {
        LOGTE(m_label.c_str(), "In Transmit: Unable to transmit message");
        return;
    }
}

bool Module::Transmit(const uint8_t* buff, size_t len, uint32_t timeout) {
    Transmit(buff, len);
    return WaitUntilTransmissionComplete(timeout);
}

void Module::Transmit(const std::string& msg) {
    Transmit(msg.c_str(), msg.size());
}

void Module::Transmit(const std::vector<uint8_t>& msg) {
    Transmit((const char*)msg.data(), msg.size());
}

[[maybe_unused]] void Module::VTransmit(const char* fmt, ...) {
    static char buff[256];

    va_list args;
    va_start(args, fmt);
    size_t len = vsnprintf(buff, sizeof(buff), fmt, args);
    va_end(args);

    Transmit(buff, len);
}

size_t Module::Receive(uint8_t* buf, uint8_t len) {
    return m_rxCirc.read(buf, len);
}

size_t Module::Receive(uint8_t* buf, uint8_t len, uint32_t timeout) {
    uint32_t deadline = HAL_GetTick() + timeout;
    while (HAL_GetTick() < deadline) {
        m_rxCirc.dmaCounter(__HAL_DMA_GET_COUNTER(m_handle->hdmarx));
        if (m_rxCirc.size() >= len) {
            return Receive(buf, len);
        }
    }
    return 0;
}

Frame Module::Receive() {
    Frame frame;
    if (m_rxFrames.size() != 0) {
        frame = m_rxFrames.front();
        m_rxFrames.pop();
    }
    return frame;
}

void Module::SetExpectedRxLen(size_t len) {
    m_efl = len;
    SetTriage();
    // ResizeDmaBuffer();
}

void Module::ClearExpectedRxLen() {
    m_efl = 0;
    SetTriage();
    // ResizeDmaBuffer();
}

void Module::SetFrameReceiveCpltCallback(const std::function<void()>& cb) {
    CEP_ASSERT(cb != nullptr, "[%s]: In SetFrameReceiveCpltCallback, invalid callback function", m_label.c_str());
    m_cb = cb;
}

void Module::ClearFrameReceiveCpltCallback() {
    m_cb = std::function<void()>();
}

void Module::SetStartOfFrameSequence(const std::string& sof) {
    m_sof = sof;
    SetTriage();
}

void Module::SetStartOfFrameSequence(const std::vector<uint8_t>& sof) {
    SetStartOfFrameSequence(std::string(sof.begin(), sof.end()));
}

void Module::SetStartOfFrameSequence(uint8_t* sof, size_t len) {
    CEP_ASSERT(sof != nullptr, "[%s]: In SetStartOfFrameSequence, sof is NULL", m_label.c_str());
    CEP_ASSERT(len > 0, "[%s]: In SetStartOfFrameSequence, len is 0", m_label.c_str());
    SetStartOfFrameSequence(std::string(sof, sof + len));
}

void Module::ClearStartOfFrameSequence() {
    m_sof = "";
    SetTriage();
}

void Module::SetEndOfFrameSequence(const std::string& eof) {
    m_eof = eof;
    SetTriage();
}

void Module::SetEndOfFrameSequence(const std::vector<uint8_t>& eof) {
    SetEndOfFrameSequence(std::string{eof.begin(), eof.end()});
}

void Module::SetEndOfFrameSequence(uint8_t* eof, size_t len) {
    CEP_ASSERT(eof != nullptr, "[%s]: In SetEndOfFrameSequence, eof is NULL", m_label.c_str());
    CEP_ASSERT(len > 0, "[%s]: In SetEndOfFrameSequence, len is 0", m_label.c_str());
    SetEndOfFrameSequence(std::string(eof, eof + len));
}

void Module::ClearEndOfFrameSequence() {
    m_eof = "";
    SetTriage();
}

void Module::FlushRecv() {
    m_rxCirc.dmaCounter(__HAL_DMA_GET_COUNTER(m_handle->hdmarx));
    m_rxCirc.setReadPos(m_rxl - __HAL_DMA_GET_COUNTER(m_handle->hdmarx) - 1);
}

/*************************************************************************************************/
/* Private method definitions */
/*************************************************************************************************/
bool Module::WaitUntilTransmissionComplete(uint32_t timeout) {
    uint32_t timeoutTime = HAL_GetTick() + timeout;

    while (HAL_GetTick() < timeoutTime) {
        if (m_handle->gState == HAL_UART_STATE_READY) {
            return true;
        }
    }

    return false;
}

bool Module::ResizeDmaBuffer() {
    HAL_StatusTypeDef s = HAL_UART_DMAStop(m_handle);
    if (s != HAL_OK) {
        LOGTE(m_label.c_str(), "Unable to stop the DMA stream! %i", (int)s);
        return false;
    }

    if (m_efl != 0) {
        m_rxl = m_efl;
        m_rxBuff.resize(m_efl);
        m_rxCirc.init(m_efl);
    }

    s = HAL_UART_Receive_DMA(m_handle, m_rxBuff.data(), m_rxl);
    if (s != HAL_OK) {
        LOGTE(m_label.c_str(), "Unable to start the DMA stream! %i", (int)s);
        return false;
    }

    return true;
}

void Module::SetTriage() {
    static constexpr size_t    lim_xof = 5;

    // LEN
    if (m_efl != 0) {
        m_triage = [&]() {
            while (m_efl <= m_rxCirc.size()) {
                std::vector<uint8_t> data;
                data.resize(m_efl);
                m_rxCirc.read(data.data(), m_efl);
                m_rxFrames.push({data, HAL_GetTick()});
                m_cb();
            }
        };
    }
    // SOF
    else if (!m_sof.empty() && m_eof.empty()) {
        m_triage = [&]() {
            std::vector<uint8_t> data(m_rxCirc.size());
            std::vector<size_t> sofs;
            m_rxCirc.peek(data.data(), m_rxCirc.size());
            SearchFrame(data, std::vector<uint8_t>{m_sof.begin(), m_sof.end()}, sofs, lim_xof);

            // Enough sof for adding frames
            if (sofs.size() > 1) {
                for (size_t i = 0; i < sofs.size() - 1; ++i) {
                    m_rxFrames.push(
                      {std::vector<uint8_t>(data.begin() + sofs[i] + m_sof.size(), data.begin() + sofs[i + 1]),
                       HAL_GetTick()});
                }
                m_rxCirc.pop(sofs[sofs.size() - 1]);    // removed processed frames
                m_cb();
            }
        };
    }
    // EOF
    else if (m_sof.empty() && !m_eof.empty()) {
        m_triage = [&]() {
            std::vector<uint8_t> data(m_rxCirc.size());
            std::vector<size_t> eofs;
            data.resize(m_rxCirc.size());
            m_rxCirc.peek(data.data(), m_rxCirc.size());
            SearchFrame(data, {m_eof.begin(), m_eof.end()}, eofs, lim_xof);

            if (!eofs.empty()) {
                size_t lastEof = 0;
                for (auto eof : eofs) {
                    m_rxFrames.push({std::vector<uint8_t>(data.begin() + lastEof, data.begin() + eof), HAL_GetTick()});
                    lastEof = eof + m_eof.size();
                }
                m_rxCirc.pop(eofs.back() + m_eof.size());    // removed processed frames
                m_cb();
            }
        };
    }
    // SOF & EOF
    else if (!m_sof.empty() && !m_eof.empty()) {
        m_triage = [&]() {
            std::vector<uint8_t> data(m_rxCirc.size());
            std::vector<size_t> sofs;
            std::vector<size_t> eofs;
            data.resize(m_rxCirc.size());
            m_rxCirc.peek(data.data(), m_rxCirc.size());
            SearchFrame(data, {m_sof.begin(), m_sof.end()}, sofs, lim_xof);
            SearchFrame(data, {m_eof.begin(), m_eof.end()}, eofs, lim_xof);
            std::vector<Interval> intervals;
            for (auto start : sofs) {
                for (auto end : eofs) {
                    if (end < start) {
                        continue;
                    }
                    if (!intervals.empty() && intervals.back().end) {
                        intervals.pop_back();
                    }
                    intervals.emplace_back(start, end);
                    break;
                }
            }
            for (auto interval : intervals) {
                m_rxFrames.push(
                  {std::vector<uint8_t>(data.begin() + interval.start + m_sof.size(), data.begin() + interval.end),
                   HAL_GetTick()});
            }
            if (!intervals.empty()) {
                m_rxCirc.pop(eofs.back() + m_eof.size());
                m_cb();
            }
        };
    }
    // No triage
    else {
        LOGTW(m_label.c_str(), "Triage disabled");
        LOGTD(m_label.c_str(), "LEN: %d", m_efl);
        LOGTD(m_label.c_str(), "SOF: %s", m_sof.empty() ? "[None]" : m_sof);
        LOGTD(m_label.c_str(), "EOF: %s", m_eof.empty() ? "[None]" : m_eof);
        m_triage = [&]() {
        };
    }
}

void Module::SearchFrame(
  std::vector<uint8_t> data,
  std::vector<uint8_t> pattern,
  std::vector<size_t>& result,
  size_t               max_depth,
  size_t               offset) {
    auto found = std::ranges::search(std::ranges::subrange(data.begin() + offset, data.end()), pattern);
    if (found.empty())
        return;

    auto start = std::distance(data.begin(), found.begin());
    auto end   = std::distance(data.begin(), found.end());
    result.push_back(start);

    if ((max_depth == 0) || (result.size() < max_depth))
        SearchFrame(data, pattern, result, max_depth, end);
}

/*************************************************************************************************/
/* Private functions definitions
 * --------------------------------------------------------------- */

}    // namespace Nilai::Drivers::Uart

#endif
/**
 * @}
 * @}
 */
/* ----- END OF FILE ----- */
