#include "Cesium/Systems/LoggerSink.h"
#include <AzCore/Debug/Trace.h>
#include <AzCore/std/parallel/scoped_lock.h>

namespace Cesium
{
    void LoggerSink::sink_it_([[maybe_unused]] const spdlog::details::log_msg& msg)
    {
#ifdef AZ_ENABLE_TRACING
        switch (msg.level)
        {
        case SPDLOG_LEVEL_TRACE:
            AZ_TracePrintf("Cesium", FormatMessage(msg).c_str());
            break;
        case SPDLOG_LEVEL_DEBUG:
            AZ_TracePrintf("Cesium", FormatMessage(msg).c_str());
            break;
        case SPDLOG_LEVEL_INFO:
            AZ_TracePrintf("Cesium", FormatMessage(msg).c_str());
            break;
        case SPDLOG_LEVEL_WARN:
            AZ_Warning("Cesium", false, FormatMessage(msg).c_str());
            break;
        case SPDLOG_LEVEL_ERROR:
            AZ_Error("Cesium", false, FormatMessage(msg).c_str());
            break;
        case SPDLOG_LEVEL_CRITICAL:
            AZ_Error("Cesium", false, FormatMessage(msg).c_str());
            break;
        default:
            AZ_TracePrintf("Cesium", FormatMessage(msg).c_str());
        }
#endif // AZ_ENABLE_TRACING
    }

    void LoggerSink::flush_()
    {
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
