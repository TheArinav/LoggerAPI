#include <sstream>

#include "LogEntry.hpp"

namespace src::logger_classes {
    string LogEntry::GenerateTimestamp() {
        char fmt[64];
        char buf[64];
        struct timeval tv{};
        struct tm *tm;
        gettimeofday(&tv, nullptr);
        tm = localtime(&tv.tv_sec);
        strftime(fmt, sizeof(fmt), "%H:%M:%S:%%06u", tm);
        snprintf(buf, sizeof(buf), fmt, tv.tv_usec);
        return buf;
    }

    string LogEntry::SerializeSeverity(Severity s) {
        switch (s) {
            case Severity::UNSPEC:
                return {};
            case Severity::DEBUG:
                return "DEBUG";
            case Severity::INFO:
                return "INFO";
            case Severity::WARNING:
                return "WARNING";
            case Severity::ERROR:
                return "ERROR";
            case Severity::CRITICAL:
                return "CRITICAL";
        }

    }

    string LogEntry::Serialize() const {
        stringstream ss{};
        ss << TimeStamp
           << ":["
           << SerializeSeverity(SeverityType)
           << "]:("
           << Source
           << ")= "
           << Message
           << endl;
        return ss.str();
    }

    LogEntry::LogEntry(Severity severity, string source, string msg):
    SeverityType(severity),Source(move(source)),Message(move(msg)){
        TimeStamp = GenerateTimestamp();
    }
} // logger_classes