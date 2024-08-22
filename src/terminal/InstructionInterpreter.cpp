#include "InstructionInterpreter.hpp"

#include <any>
#include <iostream>
#include <string>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <memory>

#include "../logger_classes/MainLogger.hpp"
#include "../logger_classes/Enums.hpp"
#include "../logger_classes/LogEntry.hpp"
#include "../IO/FileInterface.hpp"
#include "../IO/JsonFileInterface.hpp"
#include "../IO/CustomFormatFileInterface.hpp"

#define HOME "/tmp/logger/"

using namespace std;
using namespace src::logger_classes;
using namespace src::io;

namespace src::terminal {
    shared_ptr<networking::client::ServerConnection> InstructionInterpreter::Connection= {};
    void InstructionInterpreter::HandleInstruction(const LoggerInstruction &instruction) {
        auto logger = GetLogger();
        if (!logger) {
            std::cerr << "Logger not initialized" << std::endl;
            logger=make_shared<MainLogger>();
            logger->Setup();
            cout << "Initialized Logger" << endl;
        }

        logger->Exec([instruction](src::logger_classes::MainLogger &log) -> std::any {
            // Ensure the loggers are initialized if not already
            if (!log.cur_ctf)
                log.cur_ctf = make_shared<CustomFormatFileInterface>(HOME, Connection);

            if (!log.cur_json)
                log.cur_json = make_shared<JSONFileInterface>(HOME, Connection);


            // Perform logging
            auto fileInterface = log.GetFileInterface(static_cast<FileFormat>(static_cast<int>(instruction.FType)-1));
            if (!fileInterface) {
                std::cerr << "File interface not found!" << std::endl;
                return {};
            }

            std::stringstream ss{};
            ss << getCanonicalName() << ":" << instruction.PID;
            return fileInterface->LogLine(LogEntry((logger_classes::Severity)(static_cast<int>(instruction.Type) + 1),
                                                   ss.str(), instruction.Data),
                                          (TransportMode)(static_cast<int>(instruction.TMode)));
        });
    }


    std::shared_ptr<src::logger_classes::MainLogger> InstructionInterpreter::GetLogger() {
        // Assuming logger is a valid shared_ptr initialized elsewhere
        return MainLogger::Instance;
    }

    string InstructionInterpreter::getCanonicalName() {
        char hostname[1024];
        hostname[1023] = '\0';

        // Get the hostname
        if (gethostname(hostname, sizeof(hostname)) != 0) {
            perror("gethostname");
            return "";
        }

        // Resolve the hostname to get canonical name
        addrinfo hints{}, *info, *p;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(hostname, nullptr, &hints, &info) != 0) {
            perror("getaddrinfo");
            return "";
        }

        string canonicalName = hostname; // Default to hostname if no canonical name is found

        for (p = info; p != nullptr; p = p->ai_next) {
            char host[1024];
            if (getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), nullptr, 0, NI_NAMEREQD) == 0) {
                canonicalName = string(host);
                break;
            }
        }

        freeaddrinfo(info);

        return {canonicalName};
    }
} // terminal