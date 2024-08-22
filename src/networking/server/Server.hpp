#ifndef LOGGEREXAMPLE_SERVER_HPP
#define LOGGEREXAMPLE_SERVER_HPP

#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <iostream>

#include "../transport/ServerRequest.hpp"
#include "../transport/ClientResponse.hpp"
#include "../transport/Enums.hpp"
#include "../transport/Constants.hpp"

using namespace std;
using namespace src::networking::transport;

namespace src::networking::server {
    class Server {
    public:
        string ServerName;
        int FD;
        int EpollFD;
        vector<transport::ServerRequest> Instructions = {};

        explicit Server(string serverName) : ServerName(std::move(serverName)), FD(-1), EpollFD(-1) {}

        ~Server() {
            if (FD != -1) close(FD);
            if (EpollFD != -1) close(EpollFD);
        }

        void Initialize() ;

        void Run() ;

    private:
        void HandleIncomingData(int fd) ;

        void Cleanup();
    };
} // server

#endif //LOGGEREXAMPLE_SERVER_HPP
