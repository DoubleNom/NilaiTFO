/**
 * @addtogroup fileLoggerModule.cpp
 * @{
 *******************************************************************************
 * @file	fileLoggerModule.cpp
 * @author	Samuel Martel
 * @brief
 * Created on: Apr 9, 2021
 *******************************************************************************
 */
#ifdef NILAI_USE_FILE_LOGGER

#include "fileLoggerModule.h"

#include "defines/macros.hpp"
#include "services/logger.hpp"

#include <cstring>
#include <utility>

FileLogger::FileLogger(std::string label, const std::string &path) : m_label(std::move(label)), m_path(path) {
    m_logFile = cep::Filesystem::File(path, cep::Filesystem::FileModes::WRITE_APPEND);
    Log("[%s]: Initialized", m_label.c_str());
}

FileLogger::~FileLogger() {
    if (!m_path.empty()) {
        // Just flush the cache if the path is valid (not empty).
        Flush();
    }
}

bool FileLogger::DoPost() {
    if (!m_logFile.IsOpen()) {
        if (m_logFile.Open(m_path, cep::Filesystem::FileModes::WRITE_APPEND) != cep::Filesystem::Result::Ok) {
            Log("[%s]: unable to open log file: %s", m_label.c_str(),
                cep::Filesystem::ResultToStr(m_logFile.GetError()).c_str());
            return false;
        }
    }
    cep::Filesystem::Result r = m_logFile.Close();
    if (r != cep::Filesystem::Result::Ok) {
        Log("[%s]: unable to close log file: %s", m_label.c_str(), cep::Filesystem::ResultToStr(r).c_str());
        return false;
    }

    return true;
}

void FileLogger::Run() {
    static size_t lastSync = 0;

    if (HAL_GetTick() >= lastSync + SYNC_TIME) {
        lastSync = HAL_GetTick();
        Flush();
        return;
    }
}

void FileLogger::Flush() {
    using namespace cep::Filesystem;
    Result r;

    // If the cache is empty, no need to go through all that.
    if (m_cacheLoc == 0) return;


    // If the log file is not open:
    if (!m_logFile.IsOpen()) {
        // Open it.
        r = m_logFile.Open(m_path, FileModes::WRITE_APPEND);
        if (r != Result::Ok) {
            m_cacheLoc = 0;
            Log("Unable to open log file!");
            return;
        }
    }

    // Write the local cache to the file.
    size_t dw = 0;
    r = m_logFile.Write(m_cache, m_cacheLoc, &dw);
    if ((r != Result::Ok) || (m_cacheLoc != dw)) {
        m_cacheLoc = 0;
        Log("Unable to write to log file: %s", ResultToStr(r).c_str());
        return;
    }

    m_cacheLoc = 0;
}

void FileLogger::Write(const char *msg, std::size_t len) {
    if (m_cacheLoc + len >= CACHE_SIZE - 1) {
        Flush();
    }
    std::strncpy(m_cache + m_cacheLoc, msg, len);
    m_cacheLoc += len;
}

void FileLogger::Log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    VLog(fmt, args);
    va_end(args);
}

void FileLogger::VLog(const char *fmt, va_list args) {
    if (m_uart != nullptr) {
        char buffer[1024];
        int size = vsnprintf(buffer, 1024, fmt, args);
        HAL_UART_Transmit_IT(m_uart, reinterpret_cast<uint8_t *>(buffer), size);
    }
}

#endif
