#ifndef LOGGEREXAMPLE_JSONFILEINTERFACE_HPP
#define LOGGEREXAMPLE_JSONFILEINTERFACE_HPP

#include "nlohmann/json.hpp"
#include "FileInterface.hpp"


using namespace nlohmann;

namespace src::io {
    class JSONFileInterface : public FileInterface {
        bool LogLine(logger_classes::LogEntry entry, logger_classes::TransportMode TMode) override;

    public:
        JSONFileInterface();
        JSONFileInterface(string home, const shared_ptr<networking::client::ServerConnection>& remoteHost);
    };

} // io

#endif //LOGGEREXAMPLE_JSONFILEINTERFACE_HPP
