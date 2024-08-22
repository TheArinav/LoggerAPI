#ifndef LOGGEREXAMPLE_LOGENTRY_HPP
#define LOGGEREXAMPLE_LOGENTRY_HPP

#include <string>
#include <csignal>

using namespace std;

namespace src::logger_classes {
    enum class Severity{
        UNSPEC=0,
        DEBUG=1,
        INFO=2,
        WARNING=3,
        ERROR=4,
        CRITICAL=5
    };
    class LogEntry {
    public:
        LogEntry(Severity severity, string source, string msg);
        Severity SeverityType;
        string TimeStamp;
        string Source;
        string Message;
        static string SerializeSeverity(Severity s);
        static string GenerateTimestamp();
        [[nodiscard]] string Serialize() const;
    };

} // logger_classes

#endif //LOGGEREXAMPLE_LOGENTRY_HPP
