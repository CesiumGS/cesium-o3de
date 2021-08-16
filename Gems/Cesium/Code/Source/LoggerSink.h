#pragma once

#include <AzCore/std/parallel/mutex.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/logger.h>
#include <string>

namespace Cesium
{
    class LoggerSink : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
    {
    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override;

        void flush_() override;

    private:
        std::string FormatMessage(const spdlog::details::log_msg& msg);

        AZStd::mutex m_formatMutex;
    };
} // namespace Cesium
