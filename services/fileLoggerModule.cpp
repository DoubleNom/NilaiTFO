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
#if defined(NILAI_USE_FILE_LOGGER)
#include "fileLoggerModule.h"

#include "services/logger.hpp"

#include <cstring>
#include "defines/macros.hpp"

FileLogger::FileLogger(const std::string& label, const std::string& path)
: m_label(label)
, m_path(path) {
    m_logFile = cep::Filesystem::File(path, cep::Filesystem::FileModes::WRITE_APPEND);

    LOGTI(m_label.c_str(), "Initialized");
}

FileLogger::~FileLogger() {
    if (m_path.empty() == false) {
        // Just flush the cache if the path is valid (not empty).
        Flush();
    }
}

bool FileLogger::DoPost() {
    if (m_logFile.IsOpen() == false) {
        if (m_logFile.Open(m_path, cep::Filesystem::FileModes::WRITE_APPEND) != cep::Filesystem::Result::Ok) {
            LOGTE(
              m_label.c_str(),
              "POST error, unable to open log file: %s",
              cep::Filesystem::ResultToStr(m_logFile.GetError()).c_str());
            return false;
        }
    }
    cep::Filesystem::Result r = m_logFile.Close();
    if (r != cep::Filesystem::Result::Ok) {
        LOGTE(m_label.c_str(), "POST error, unable to close log file: %s", cep::Filesystem::ResultToStr(r));
        return false;
    }
    LOGTI(m_label.c_str(), "POST OK!");
    return true;
}

void FileLogger::Run() {
    static size_t lastSync = 0;

    if (HAL_GetTick() >= lastSync + SYNC_TIME) {
        lastSync = HAL_GetTick();
        Flush();
    }
}

void FileLogger::Flush() {
    using namespace cep::Filesystem;
    Result r;

    // If the cache is empty, no need to go through all that.
    if (m_cacheLoc == 0) {
        return;
    }

    // If the log file is not open:
    if (m_logFile.IsOpen() == false) {
        // Open it.
        r = m_logFile.Open(m_path, FileModes::WRITE_APPEND);
        if (r != Result::Ok) {
            CEP_ASSERT(false, "Unable to open log file!");
            m_cacheLoc = 0;
            return;
        }
    }

    // Write the local cache to the file.
    size_t dw = 0;
    r         = m_logFile.Write(m_cache, m_cacheLoc, &dw);
    if ((r != Result::Ok) || (m_cacheLoc != dw)) {
        CEP_ASSERT(false, "Unable to write to log file: %s", ResultToStr(r).c_str());
        m_cacheLoc = 0;
        return;
    }

    r = m_logFile.Close();
    if (r != Result::Ok) {
        CEP_ASSERT(false, "Unable to close file!");
        m_cacheLoc = 0;
        return;
    }

    m_cacheLoc = 0;
}

void FileLogger::Log(const char* msg, size_t len) {
    // If buffer is full, flush it.
    if (m_cacheLoc + len >= CACHE_SIZE - 1) {
        Flush();
    }
    std::strncpy(&m_cache[m_cacheLoc], msg, len);
    m_cacheLoc += len;
}

#endif
