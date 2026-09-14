#pragma once

#include <chrono>
#include <deque>
#include <fstream>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace LavaEngine
{
    enum class LogLevel
    {
        Trace,
        Info,
        Warning,
        Error,
        Fatal,
        Debug,
    };

    struct LogEntry
    {
        LogLevel level;
        std::string timestamp;
        std::string message;
    };

    class Logger
    {
    public:
        // Logs to the default console stream (std::cout).
        Logger();

        // Logs to the given file in addition to the console.
        explicit Logger(const std::string& filename);

        ~Logger();

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        // Only messages at or above this level are emitted.
        void setLevel(LogLevel level);

        [[nodiscard]]
        LogLevel level() const;

        void setConsoleEnabled(bool enabled);

        void setFileEnabled(bool enabled);

        // Redirect console output to a custom stream.
        void setOutput(std::ostream& stream);

        template <typename... Args>
        void trace(Args&&... args)
        {
            log(LogLevel::Trace, buildMessage(std::forward<Args>(args)...));
        }

        template <typename... Args>
        void info(Args&&... args)
        {
            log(LogLevel::Info, buildMessage(std::forward<Args>(args)...));
        }

        template <typename... Args>
        void warning(Args&&... args)
        {
            log(LogLevel::Warning, buildMessage(std::forward<Args>(args)...));
        }

        template <typename... Args>
        void error(Args&&... args)
        {
            log(LogLevel::Error, buildMessage(std::forward<Args>(args)...));
        }

        template <typename... Args>
        void fatal(Args&&... args)
        {
            log(LogLevel::Fatal, buildMessage(std::forward<Args>(args)...));
        }

        template <typename... Args>
        void debug(Args&&... args)
        {
            log(LogLevel::Debug, buildMessage(std::forward<Args>(args)...));
        }

        // Emits a pre-formatted message.
        void log(LogLevel level, const std::string& message);

        // Maximum number of messages kept in the in-memory history.
        static constexpr std::size_t kDefaultHistorySize = 1000;

        void setHistorySize(std::size_t size);

        void clearHistory();

        // Thread-safe snapshot of the buffered messages.
        [[nodiscard]]
        std::vector<LogEntry> entries() const;

        // Shared application-wide logger.
        static Logger& instance();

    private:
        static std::string_view toString(LogLevel level);
        static std::string_view toColor(LogLevel level);
        static std::string timestamp();

        template <typename... Args>
        static std::string buildMessage(Args&&... args)
        {
            std::ostringstream stream;
            (stream << ... << std::forward<Args>(args));
            return stream.str();
        }

        LogLevel m_level = LogLevel::Trace;
        bool m_consoleEnabled = true;
        bool m_fileEnabled = false;

        mutable std::mutex m_mutex;
        std::ostream* m_console = nullptr;
        std::ofstream m_file;

        std::deque<LogEntry> m_history;
        std::size_t m_historySize = kDefaultHistorySize;
    };
}