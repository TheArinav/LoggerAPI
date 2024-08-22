// JSONFileInterface.cpp
#include "JsonFileInterface.hpp"
#include <fstream>
#include <iostream>
#include <unistd.h>

namespace src::io {

    bool JSONFileInterface::LogLine(logger_classes::LogEntry entry, logger_classes::TransportMode TMode) {
        if(EntryCount == MAX_ENTRIES)
            return false;
        ++EntryCount;

        json j{};
        j["Timestamp"] = entry.TimeStamp;
        j["Severity"] = logger_classes::LogEntry::SerializeSeverity(entry.SeverityType);
        j["Source"] = entry.Source;
        j["Message"] = entry.Message;

        if (TMode == logger_classes::TransportMode::Local) {
            std::ofstream stream(HomeDirectory + FileName, std::ios::app);  // Open in append mode
            if (!stream.is_open()) {
                std::cerr << "Failed to open file: " << HomeDirectory + FileName << std::endl;
                return false;
            }
            stream << j.dump(4) << std::endl;
        } else {
            if (!RemoteHost) {
                std::cerr << "Remote host not set" << std::endl;
                return false;
            }
            RemoteHost->Request(networking::transport::ServerRequest(logger_classes::FileFormat::JSON,
                                                                     entry.SeverityType, getpid(),
                                                                     entry.Serialize()));
        }
        return true;
    }

    JSONFileInterface::JSONFileInterface(string home, const shared_ptr<networking::client::ServerConnection>& remoteHost) :
            FileInterface(std::move(home), ".json", remoteHost) {
    }

    JSONFileInterface::JSONFileInterface() : FileInterface() {}

} // namespace src::io
