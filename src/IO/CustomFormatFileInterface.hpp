#ifndef LOGGEREXAMPLE_CUSTOMFORMATFILEINTERFACE_HPP
#define LOGGEREXAMPLE_CUSTOMFORMATFILEINTERFACE_HPP

#include "FileInterface.hpp"

namespace src::io {

    class CustomFormatFileInterface : public FileInterface {
    public:
        CustomFormatFileInterface();

        CustomFormatFileInterface(string home, const shared_ptr<networking::client::ServerConnection>& remoteHost);
    private:
        bool LogLine(logger_classes::LogEntry entry, logger_classes::TransportMode TMode) override;
    };

} // io

#endif //LOGGEREXAMPLE_CUSTOMFORMATFILEINTERFACE_HPP
