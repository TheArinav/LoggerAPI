#include "Server.hpp"

#include "../../terminal/InstructionInterpreter.hpp"

#define PORT "25480"

namespace src::networking::server {

    void Server::Initialize() {
        // Create UDP socket
        FD = socket(AF_INET, SOCK_DGRAM, 0);
        if (FD == -1) {
            cerr << "Failed to create socket: " << strerror(errno) << endl;
            return;
        }

        // Allow socket to reuse address
        int opt = 1;
        if (setsockopt(FD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            cerr << "Failed to set socket options: " << strerror(errno) << endl;
            return;
        }

        // Set the socket to non-blocking mode
        int flags = fcntl(FD, F_GETFL, 0);
        if (flags == -1 || fcntl(FD, F_SETFL, flags | O_NONBLOCK) == -1) {
            cerr << "Failed to set non-blocking mode: " << strerror(errno) << endl;
            return;
        }

        // Bind the socket to an address and port
        sockaddr_in serverAddr{};
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddr.sin_port = htons(atoi(PORT));

        if (bind(FD, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            cerr << "Failed to bind socket: " << strerror(errno) << endl;
            return;
        }

        // Create epoll instance
        EpollFD = epoll_create1(0);
        if (EpollFD == -1) {
            cerr << "Failed to create epoll instance: " << strerror(errno) << endl;
            return;
        }

        // Add the socket to epoll
        epoll_event event{};
        event.events = EPOLLIN;
        event.data.fd = FD;
        if (epoll_ctl(EpollFD, EPOLL_CTL_ADD, FD, &event) == -1) {
            cerr << "Failed to add socket to epoll: " << strerror(errno) << endl;
            return;
        }

        cout << "Server initialized and listening on port " << PORT << endl;
    }

    void Server::Run() {
        const int MAX_EVENTS = 10;
        epoll_event events[MAX_EVENTS];
        int retryCount = 0;
        const int maxRetries = 5;

        while (true) {
            int eventCount = epoll_wait(EpollFD, events, MAX_EVENTS, -1);
            if (eventCount == -1) {
                cerr << "epoll_wait error: " << strerror(errno) << endl;
                if (errno == EINTR) {
                    // Retry if interrupted by a signal
                    if (++retryCount > maxRetries) break;
                    continue;
                }
                break;
            }
            retryCount = 0;

            for (int i = 0; i < eventCount; ++i) {
                if (events[i].events & EPOLLIN) {
                    HandleIncomingData(events[i].data.fd);
                }
            }
        }

        Cleanup();
    }

    void Server::HandleIncomingData(int fd) {
        char buffer[1024];
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);

        ssize_t receivedBytes = recvfrom(fd, buffer, sizeof(buffer) - 1, 0,
                                         (sockaddr*)&clientAddr, &clientAddrLen);

        if (receivedBytes > 0) {
            buffer[receivedBytes] = '\0';
            std::string data(buffer);

            std::cout << "Received data: " << data << std::endl;

            // Process the received data (deserialize, handle, etc.)
            try {
                auto request = ServerRequest::Deserialize(data);
                terminal::InstructionInterpreter::HandleInstruction(terminal::LoggerInstruction(
                        (terminal::InstructionType)(static_cast<int>(request.Severity) - 1),
                        (terminal::FormatType)(static_cast<int>(request.Format) + 1),
                        terminal::TransMode::Local,
                        request.Origin,
                        request.Data));

                ClientResponse response(true, "Request processed successfully");

                // Send response back to client
                std::string responseData = response.Serialize();
                sendto(fd, responseData.c_str(), responseData.size(), 0,
                       (sockaddr*)&clientAddr, clientAddrLen);
            } catch (const std::exception &e) {
                std::cerr << "Error processing incoming data: " << e.what() << std::endl;
            }
        }
    }

    void Server::Cleanup() {
        if (FD != -1) {
            close(FD);
            FD = -1;
        }
        if (EpollFD != -1) {
            close(EpollFD);
            EpollFD = -1;
        }
        cout << "Cleaned up resources." << endl;
    }

} // server
