#include "LoggerSink.h"
#include <AzCore/Console/ILogger.h>
#include <AzCore/std/parallel/scoped_lock.h>

namespace Cesium
{
    void LoggerSink::sink_it_(const spdlog::details::log_msg& msg)
    {
        switch (msg.level)
        {
        case SPDLOG_LEVEL_TRACE:
            AZLOG_TRACE(FormatMessage(msg).c_str());
            break;
        case SPDLOG_LEVEL_DEBUG:
            AZLOG_DEBUG(FormatMessage(msg).c_str());
            break;
        case SPDLOG_LEVEL_INFO:
            AZLOG_INFO(FormatMessage(msg).c_str());
            break;
        case SPDLOG_LEVEL_WARN:
            AZLOG_WARN(FormatMessage(msg).c_str());
            break;
        case SPDLOG_LEVEL_ERROR:
            AZLOG_ERROR(FormatMessage(msg).c_str());
            break;
        case SPDLOG_LEVEL_CRITICAL:
            AZLOG_FATAL(FormatMessage(msg).c_str());
            break;
        default:
            AZLOG_INFO(FormatMessage(msg).c_str());
        }
    }

    void LoggerSink::flush_()
    {
        AZLOG_FLUSH();
    }

    std::string LoggerSink::FormatMessage(const spdlog::details::log_msg& msg)
    {
        // Frustratingly, spdlog::formatter isn't thread safe. So even though our sink
        // itself doesn't need to be protected by a mutex, the formatter does.
        // See https://github.com/gabime/spdlog/issues/897
        AZStd::scoped_lock<AZStd::mutex> lock(m_formatMutex);

        spdlog::memory_buf_t formatted;
        formatter_->format(msg, formatted);
        return fmt::to_string(formatted);
    }
} // namespace Cesium
