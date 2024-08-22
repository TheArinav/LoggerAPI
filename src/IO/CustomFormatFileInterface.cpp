// CustomFormatFileInterface.cpp
#include <fstream>
#include <unistd.h>
#include <iostream>
#include "CustomFormatFileInterface.hpp"

namespace src::io {
    CustomFormatFileInterface::CustomFormatFileInterface(string home, const shared_ptr<networking::client::ServerConnection>& remoteHost)
            : FileInterface(std::move(home), ".ctf", remoteHost) {}

    CustomFormatFileInterface::CustomFormatFileInterface() : FileInterface() {}

    bool CustomFormatFileInterface::LogLine(logger_classes::LogEntry entry, logger_classes::TransportMode TMode) {
        if (EntryCount == MAX_ENTRIES)
            return false;
        ++EntryCount;
        if (TMode == logger_classes::TransportMode::Local) {
            std::ofstream stream(HomeDirectory + FileName, std::ios::app);  // Open in append mode
            if (!stream.is_open()) {
                std::cerr << "Failed to open file: " << HomeDirectory + FileName << std::endl;
                return false;
            }
            stream << entry.Serialize() << std::endl;
        } else if (TMode == logger_classes::TransportMode::Remote) {
            if (!RemoteHost) {
                std::cerr << "Remote host not set" << std::endl;
                return false;
            }
            RemoteHost->Request(networking::transport::ServerRequest(logger_classes::FileFormat::Custom,
                                                                     entry.SeverityType, getpid(),
                                                                     entry.Serialize()));
        }
        return true;
    }
} // namespace src::io
