#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "FileInterface.hpp"

#include "../logger_classes/Enums.hpp"

namespace src::io {
    int FileInterface::FileCount=0;
    FileInterface::FileInterface(string home, const string &extension,
                                 const shared_ptr<networking::client::ServerConnection>& remoteHost) :
            HomeDirectory(move(home)), RemoteHost(remoteHost) {
        stringstream ss{};
        ss << "log" << FileCount << FILE_SUB_EXTENSION << extension;
        FileName = ss.str();
        ++FileCount;
        system("mkdir -p /tmp/logger");
        if(!(FileStream=ofstream(HomeDirectory + FileName)).good())
        {
            FileStream.close();
            perror("Failed to access file");
            exit(EXIT_FAILURE);
        }
        FileStream.close();
    }

    void FileInterface::ReflectLog(const string& path, logger_classes::TransportMode TMode) {
        if(TMode == logger_classes::TransportMode::Local) {
            if (!filesystem::copy_file(HomeDirectory + FileName, path))
                cerr << "Failed to reflect file";
        }else if (TMode == logger_classes::TransportMode::Remote){
            if (!RemoteHost->Transfer(HomeDirectory+FileName,path))
                cerr << "Failed to reflect file";
        }
    }

} // io