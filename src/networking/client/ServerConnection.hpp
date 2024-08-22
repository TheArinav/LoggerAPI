//
// Created by IMOE001 on 8/14/2024.
//

#ifndef LOGGEREXAMPLE_SERVERCONNECTION_HPP
#define LOGGEREXAMPLE_SERVERCONNECTION_HPP

#include <string>
#include "../transport/ClientResponse.hpp"
#include "../transport/Enums.hpp"
#include "../transport/Constants.hpp"
#include "../transport/ServerRequest.hpp"

#define PORT "25480"

using namespace std;

namespace src::networking::client {
            class ServerConnection {
            public:
                ServerConnection(const ServerConnection& other);
                ServerConnection()=default;
                ServerConnection(string  p_prof, string  uname, string  ip);
                string PuTTYProfile;
                string UserName;
                string IP;
                int FD{-1};
                transport::ClientResponse Request(const transport::ServerRequest& request);
                bool Transfer(const string &org, const string &dest);
            };
        } // client

#endif //LOGGEREXAMPLE_SERVERCONNECTION_HPP
