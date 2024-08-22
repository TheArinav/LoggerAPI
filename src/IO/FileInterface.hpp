#ifndef LOGGEREXAMPLE_FILEINTERFACE_HPP
#define LOGGEREXAMPLE_FILEINTERFACE_HPP

#include <string>
#include <fstream>
#include <memory>
#include "../logger_classes/Enums.hpp"
#include "../logger_classes/LogEntry.hpp"
#include "../networking/client/ServerConnection.hpp"

using namespace std;

namespace src::io {

    class FileInterface {
    public:
        FileInterface()=default;
        explicit FileInterface(string home, const string &extension,
                               const shared_ptr<networking::client::ServerConnection>& remoteHost);
        virtual bool LogLine(logger_classes::LogEntry entry, logger_classes::TransportMode TMode)=0;
        void ReflectLog(const string& path, logger_classes::TransportMode TMode);

        static const int MAX_ENTRIES = 32768;
        const string FILE_SUB_EXTENSION = ".logger";
        static int FileCount;
        int EntryCount=0;
        string HomeDirectory;
        string FileName;
        shared_ptr<networking::client::ServerConnection> RemoteHost{};
        ofstream FileStream;
    };

} // io

#endif //LOGGEREXAMPLE_FILEINTERFACE_HPP
