/**
 * @addtogroup  defines
 * @{
 * @addtogroup  logger
 * @{
 *
 * @file        Logger.hpp
 * @author      Samuel Martel
 * @date        2020/06/29 - 11:30
 */
#pragma once

#if defined(NILAI_USE_LOGGER)

/*************************************************************************************************/
/* Includes
 * ------------------------------------------------------------------------------------
 */
#include "../defines/internalConfig.h"

#include <cstdarg>    // For va_list
#include <functional>

#ifdef NILAI_LOGGER_HEADER_DISABLE_TIME
#define LOG_TIME_FORMAT
#define LOG_TIME_ARGS
#elif defined(NILAI_LOGGER_USE_RTC)
#ifndef NILAI_USE_RTC
#error NILAI_LOGGER_USE_RTC was defined, the RTC module must also be enabled!
#endif
#include "drivers/rtcModule.h"
#define LOG_TIME_FORMAT "[%s %s.%03i]"
#define GET_DATE()      Nilai::Drivers::RtcModule::Get()->GetDate().ToStr().c_str()
#define GET_TIME()      Nilai::Drivers::RtcModule::Get()->GetTime().ToStr().c_str()
#define GET_CUR_MS()    (Nilai::GetTime())
#define LOG_TIME_ARGS   , GET_DATE(), GET_TIME(), GET_CUR_MS()
#else
#include NILAI_HAL_HEADER
#define LOG_TIME_FORMAT "[%02i:%02i:%02i.%03i]"
#define GET_CUR_H()     (GET_CUR_MIN() / 60)
#define GET_CUR_MIN()   (GET_CUR_SEC() / 60)
#define GET_CUR_SEC()   (GET_CUR_MS() / 1000)
#define GET_CUR_MS()    (HAL_GetTick())
#define LOG_TIME_ARGS   , GET_CUR_H(), GET_CUR_MIN() % 60, GET_CUR_SEC() % 60, GET_CUR_MS() % 1000
#endif

#define INT_NILAI_LOG_IMPL_OK

#ifdef NILAI_LOGGER_HEADER_TAG_MAX_SIZE
#define NILAI_STRH(x)  #x
#define NILAI_STR(x)   NILAI_STRH(x)
#define NILAI_TAG_SIZE NILAI_STR(NILAI_LOGGER_HEADER_TAG_MAX_SIZE)
#define LOG_TAG_FORMAT "[%-" NILAI_TAG_SIZE "." NILAI_TAG_SIZE "s]"
#undef STR
#undef STRH
#undef SIZE
#else
#define LOG_TAG_FORMAT "[%s]"
#endif

#ifndef NILAI_LOG_COLOR_DISABLE
#define LOG_COLOR_DEBUG NILAI_LOGGER_COLOR_DEBUG
#define LOG_COLOR_INFO  NILAI_LOGGER_COLOR_INFO
#define LOG_COLOR_WARN  NILAI_LOGGER_COLOR_WARN
#define LOG_COLOR_ERROR NILAI_LOGGER_COLOR_ERROR
#define LOG_COLOR_FATAL NILAI_LOGGER_COLOR_FATAL
#define LOG_COLOR_NONE  "\033[00m"
#else
#define LOG_COLOR_DEBUG ""
#define LOG_COLOR_INFO  ""
#define LOG_COLOR_WARN  ""
#define LOG_COLOR_ERROR ""
#define LOG_COLOR_FATAL ""
#define LOG_COLOR_NONE  ""
#endif

#ifndef NILAI_LOGGER_HEADER_LEVEL_SHORT
#define LOG_LEVEL_DEBUG "DEBUG"
#define LOG_LEVEL_INFO  "INFO "
#define LOG_LEVEL_WARN  "WARN "
#define LOG_LEVEL_ERROR "ERROR"
#define LOG_LEVEL_FATAL "FATAL"
#else
#define LOG_LEVEL_DEBUG "D"
#define LOG_LEVEL_INFO  "I"
#define LOG_LEVEL_WARN  "W"
#define LOG_LEVEL_ERROR "E"
#define LOG_LEVEL_FATAL "F"
#endif

#ifndef NILAI_LOGGER_HEADER_LEVEL_NONE
#define LOG_LEVEL_WRAPPER(level) "[" level "]"
#else
#define LOG_LEVEL_WRAPPER(level)
#endif

#define LOG_HELPER(msg, level, color, ...)                                                                             \
    Logger::Log(color LOG_TIME_FORMAT LOG_LEVEL_WRAPPER(level) " " msg              LOG_COLOR_NONE                     \
                                                               "\r\n" LOG_TIME_ARGS __VA_OPT__(, ) __VA_ARGS__)


#define LOG_HELPER_TAG(tag, msg, level, color, ...)                                                                    \
    Logger::Log(                                                                                                       \
      color LOG_TIME_FORMAT LOG_LEVEL_WRAPPER(level) LOG_TAG_FORMAT " " msg LOG_COLOR_NONE "\r\n" LOG_TIME_ARGS,       \
      tag                                                                   __VA_OPT__(, ) __VA_ARGS__)

#if defined(NILAI_LOGGER_ENABLE_DEBUG) && defined(INT_NILAI_LOG_IMPL_OK)
#define LOGD(msg, ...)       LOG_HELPER(msg, LOG_LEVEL_DEBUG, LOG_COLOR_DEBUG __VA_OPT__(, ) __VA_ARGS__)
#define LOGTD(tag, msg, ...) LOG_HELPER_TAG(tag, msg, LOG_LEVEL_DEBUG, LOG_COLOR_DEBUG __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOGD(msg, ...)
#define LOGTD(tag, msg, ...)
#endif

#if defined(NILAI_LOGGER_ENABLE_INFO) && defined(INT_NILAI_LOG_IMPL_OK)
#define LOGI(msg, ...)       LOG_HELPER(msg, LOG_LEVEL_INFO, LOG_COLOR_INFO __VA_OPT__(, ) __VA_ARGS__)
#define LOGTI(tag, msg, ...) LOG_HELPER_TAG(tag, msg, LOG_LEVEL_INFO, LOG_COLOR_INFO __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOGI(msg, ...)
#define LOGTI(tag, msg, ...)
#endif

#if defined(NILAI_LOGGER_ENABLE_WARN) && defined(INT_NILAI_LOG_IMPL_OK)
#define LOGW(msg, ...)       LOG_HELPER(msg, LOG_LEVEL_WARN, LOG_COLOR_WARN __VA_OPT__(, ) __VA_ARGS__)
#define LOGTW(tag, msg, ...) LOG_HELPER_TAG(tag, msg, LOG_LEVEL_WARN, LOG_COLOR_WARN __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOGW(msg, ...)
#define LOGTW(tag, msg, ...)
#endif

#if defined(NILAI_LOGGER_ENABLE_ERROR) && defined(INT_NILAI_LOG_IMPL_OK)
#define LOGE(msg, ...)       LOG_HELPER(msg, LOG_LEVEL_ERROR, LOG_COLOR_ERROR __VA_OPT__(, ) __VA_ARGS__)
#define LOGTE(tag, msg, ...) LOG_HELPER_TAG(tag, msg, LOG_LEVEL_ERROR, LOG_COLOR_ERROR __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOGE(msg, ...)
#define LOGTE(tag, msg, ...)
#endif

#if defined(NILAI_LOGGER_ENABLE_FATAL) && defined(INT_NILAI_LOG_IMPL_OK)
#define LOGF(msg, ...)       LOG_HELPER(msg, LOG_LEVEL_FATAL, LOG_COLOR_FATAL __VA_OPT__(, ) __VA_ARGS__)
#define LOGTF(tag, msg, ...) LOG_HELPER_TAG(tag, msg, LOG_LEVEL_FATAL, LOG_COLOR_FATAL __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOGF(msg, ...)
#define LOGTF(tag, msg, ...)
#endif

#if defined(NILAI_USE_UART)
namespace Nilai::Drivers::Uart {
class Module;
}
#endif

using LogFunc = std::function<void(const char*, size_t)>;

class Logger {
  public:
#if defined(NILAI_USE_UART)
    Logger(Nilai::Drivers::Uart::Module* uart = nullptr, const LogFunc& logFunc = {});
#else
    Logger(const LogFunc& logFunc);
#endif
    ~Logger();

    static void Log(const char* fmt, ...);
    static void VLog(const char* fmt, va_list args);

#if defined(NILAI_USE_UART)
    Nilai::Drivers::Uart::Module* GetUart() { return m_uart; }
#endif

    void SetLogFunc(const LogFunc& logFunc) { m_logFunc = logFunc; }

  private:
    static Logger* s_instance;

#if defined(NILAI_USE_UART)
    Nilai::Drivers::Uart::Module* m_uart = nullptr;
#endif
    LogFunc m_logFunc = {};
};
#endif
/*************************************************************************************************/
/* LOG_CRITICAL("Have a wonderful day! :)"); */
/**
 * @}
 * @}
 */
/* ----- END OF FILE ----- */
