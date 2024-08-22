#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <sys/epoll.h>
#include <sys/un.h>

#include "TerminalUserInterface.hpp"

#define MAX_EVENTS 16

namespace src::terminal {
    atomic<int> TerminalUserInterface::FD = -1;
    atomic<int> TerminalUserInterface::EpollFD = -1;
    atomic<bool> TerminalUserInterface::f_Stop = false;
    vector<LoggerInstruction> TerminalUserInterface::Instructions = {};

    void TerminalUserInterface::Start() {
        MainLogger::Setup();
        EpollFD.store(epoll_create1(0));
        if (EpollFD == -1) {
            cerr << "Error in epoll_create1:\n\t" << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }

        // Define a Unix socket address
        sockaddr_un server_addr{};
        memset(&server_addr, 0, sizeof(sockaddr_un));
        server_addr.sun_family = AF_UNIX;
        strncpy(server_addr.sun_path, "/tmp/your_socket_name.sock", sizeof(server_addr.sun_path) - 1);

        // Create a Unix socket
        FD.store(socket(AF_UNIX, SOCK_STREAM, 0));
        if (FD == -1) {
            perror("socket()");
            exit(EXIT_FAILURE);
        }

        int yes = 1;
        if (setsockopt(FD.load(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            cerr << "Error in setsockopt():\n\t" << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }

        // Bind to the Unix socket path
        unlink(server_addr.sun_path);  // Remove any existing file at the socket path
        if (bind(FD.load(), (struct sockaddr *) &server_addr, sizeof(sockaddr_un)) == -1) {
            perror("bind()");
            exit(EXIT_FAILURE);
        }

        if (listen(FD.load(), 10) == -1) {
            cerr << "Listen failure, cause:\n\t" << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }

        // Set the socket to non-blocking mode
        int flags = fcntl(FD.load(), F_GETFL, 0);
        if (flags == -1) {
            cerr << "Error in fcntl:\n\t" << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }
        flags |= O_NONBLOCK;
        if (fcntl(FD.load(), F_SETFL, flags) == -1) {
            cerr << "Error in fcntl:\n\t" << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }

        epoll_event event{};
        event.data.fd = FD.load();
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(EpollFD, EPOLL_CTL_ADD, FD.load(), &event) == -1) {
            cerr << "Error in epoll_ctl:\n\t" << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }

        while (!f_Stop.load()) {
            vector<epoll_event> events(MAX_EVENTS);
            int n = epoll_wait(EpollFD.load(), events.data(), MAX_EVENTS, -1);
            if (n == -1) {
                if (errno == EINTR) continue;
                perror("epoll_wait");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < n; ++i) {
                if (events[i].data.fd == FD.load()) {
                    sockaddr_storage addr{};
                    socklen_t addr_len = sizeof(addr);
                    int new_fd = accept(FD.load(), (sockaddr *) &addr, &addr_len);
                    if (new_fd == -1) {
                        perror("accept");
                        continue;
                    }

                    int flags = fcntl(new_fd, F_GETFL, 0);
                    if (flags == -1) {
                        perror("fcntl");
                        close(new_fd);
                        continue;
                    }
                    flags |= O_NONBLOCK;
                    if (fcntl(new_fd, F_SETFL, flags) == -1) {
                        perror("fcntl");
                        close(new_fd);
                        continue;
                    }

                    epoll_event event{};
                    event.data.fd = new_fd;
                    event.events = EPOLLIN | EPOLLET;
                    if (epoll_ctl(EpollFD.load(), EPOLL_CTL_ADD, new_fd, &event) == -1) {
                        perror("epoll_ctl");
                        close(new_fd);
                        continue;
                    }
                } else {
                    char buffer[1024];
                    while (true) {
                        ssize_t bytesRead = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
                        if (bytesRead > 0) {
                            string buff(buffer, bytesRead);
                            size_t pos = 0;
                            while (pos < buff.length()) {
                                size_t start = buff.find('[', pos);
                                size_t end = buff.find(']', start);
                                if (start == string::npos || end == string::npos) {
                                    break;
                                }

                                string instruction_str = buff.substr(start, end - start + 1);
                                try {
                                    LoggerInstruction instruction = LoggerInstruction::Deserialize(instruction_str);
                                    Instructions.push_back(instruction);
                                } catch (const exception &e) {
                                    cerr << "Error deserializing instruction: " << e.what() << endl;
                                }

                                pos = end + 1;
                            }
                        } else if (bytesRead == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                            break;
                        } else {
                            if (bytesRead == 0) {
                                cerr << "Client closed the connection." << std::endl;
                            } else {
                                perror("recv");
                            }
                            close(events[i].data.fd);
                            break;
                        }
                    }
                }
            }

            while (!Instructions.empty()) {
                auto cur = Instructions.front();
                Instructions.erase(Instructions.begin());
                InstructionInterpreter::HandleInstruction(cur);
            }
        }
    }

    void TerminalUserInterface::Stop() {
        if (FD != -1)
            close(FD);
        if (EpollFD != -1)
            close(EpollFD);
        f_Stop.store(true);
    }
} // namespace src::terminal
