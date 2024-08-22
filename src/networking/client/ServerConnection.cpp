// ServerConnection.cpp
#include "ServerConnection.hpp"
#include <string>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <utility>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cstring>
#include <arpa/inet.h>
#include <sstream>

using namespace std;

namespace src::networking::client {

    ServerConnection::ServerConnection(string p_prof, string uname, string ip)
            : PuTTYProfile(move(p_prof)), UserName(move(uname)), IP(move(ip)), FD(-1) {}

    ServerConnection::ServerConnection(const ServerConnection &other)
            : PuTTYProfile(other.PuTTYProfile), UserName(other.UserName), IP(other.IP), FD(-1) {
        if (other.FD != -1) {
            FD = dup(other.FD);  // Duplicate file descriptor
            if (FD == -1) {
                perror("dup");
            }
        }
    }

    bool ServerConnection::Transfer(const string& org, const string& dest) {
        stringstream ss{};
        ss << "pscp -load "
           << PuTTYProfile
           << " "
           << UserName
           << "@"
           << IP
           << ":"
           << dest
           << " "
           << org;
        array<char, 128> buf{};
        string res{};
        shared_ptr<FILE> pipe(popen(ss.str().c_str(),"r"),pclose);
        if (!pipe) {
            cerr << "Failed to open pipe for command: " << ss.str() << endl;
            return false;
        }
        while (fgets(buf.data(), buf.size(), pipe.get()) != nullptr)
            res += buf.data();
        cout << "PSCP output: \n" << res << endl;
        return true;
    }

    transport::ClientResponse ServerConnection::Request(const transport::ServerRequest& request) {
        if (FD != -1) {
            close(FD);
            FD = -1;
        }

        if (IP.empty())
            IP = "localhost";

        addrinfo hints{}, *res = nullptr, *p = nullptr;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;  // Use SOCK_DGRAM for UDP

        int stat = getaddrinfo(IP.c_str(), PORT, &hints, &res);
        if (stat != 0) {
            fprintf(stderr, "ServerConnection: getaddrinfo: %s\n", gai_strerror(stat));
            return {false, ""};
        }

        for (p = res; p; p = p->ai_next) {
            FD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (FD == -1) {
                perror("ServerConnection: socket");
                continue;
            }
            break;
        }

        freeaddrinfo(res);

        if (FD == -1) {
            return {false, ""};
        }

        auto to_send = request.Serialize();

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(atoi(PORT));

        inet_pton(AF_INET, IP.c_str(), &server_addr.sin_addr);

        ssize_t sent_bytes = sendto(FD, to_send.c_str(), to_send.size(), 0,
                                    (sockaddr*)&server_addr, sizeof(server_addr));
        if (sent_bytes == -1) {
            perror("ServerConnection: sendto");
            close(FD);
            return {false, ""};
        }

        char buf[1024];
        sockaddr_in from_addr{};
        socklen_t from_len = sizeof(from_addr);
        ssize_t recv_bytes = recvfrom(FD, buf, sizeof(buf), 0,
                                      (sockaddr*)&from_addr, &from_len);
        if (recv_bytes == -1) {
            perror("ServerConnection: recvfrom");
            close(FD);
            return {false, ""};
        }

        close(FD);

        std::string response_data(buf, recv_bytes);
        return {true, response_data};
    }

} // namespace src::networking::client
