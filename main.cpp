#include <iostream>
#include <csignal>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <chrono>
#include <sys/wait.h>
#include <netdb.h>

#include "src/terminal/TerminalUserInterface.hpp"
#include "src/networking/transport/ServerRequest.hpp"
#include "src/networking/transport/ClientResponse.hpp"
#include "src/networking/transport/Enums.hpp"
#include "src/networking/transport/Constants.hpp"
#include "src/networking/server/Server.hpp"

using namespace src::terminal;
using namespace std;

void signalHandler(int signum) {
    cout << "\n\nInterrupt signal (" << signum << ") received.\n";
    TerminalUserInterface::Stop();
    exit(signum);
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGBUS, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGPIPE, SIG_IGN);

    InstructionInterpreter::Connection = make_shared<src::networking::client::ServerConnection>();

    std::cout << "Starting program..." << std::endl;

    pid_t c_pid1 = fork();

    if (c_pid1 == -1) {
        perror("fork");
        std::cerr << "Fork failed, exiting." << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (c_pid1 > 0) {
        // Parent process: Fork the second child
        pid_t c_pid2 = fork();

        if (c_pid2 == -1) {
            perror("fork");
            std::cerr << "Second fork failed, exiting." << std::endl;
            exit(EXIT_FAILURE);
        } else if (c_pid2 > 0) {
            // Parent process
            std::cout << "main: starting TerminalUserInterface..." << std::endl;
            TerminalUserInterface::Start();

            // Wait for children processes to finish
            int status;
            waitpid(c_pid1, &status, 0);
            waitpid(c_pid2, &status, 0);
        } else {
            // Second child process (Server)
            std::cout << "p1: setting up server." << std::endl;
            auto s = src::networking::server::Server("host");
            std::cout << "p1: server created." << std::endl;
            s.Initialize();
            std::cout << "p1: server initialized." << std::endl;
            s.Run();
            exit(EXIT_SUCCESS);
        }
    }
    else {
        // First child process (Client)
        std::cout << "p0:, waiting for the server to start..." << std::endl;

        const int max_attempts = 5;
        int attempt = 0;
        int FD = -1;

        while (attempt < max_attempts) {
            std::this_thread::sleep_for(std::chrono::seconds(2));  // Wait for 2 seconds

            std::cout << "p0: Attempting to connect to server (attempt " << (attempt + 1) << ")..." << std::endl;

            // Use Unix domain socket to connect to the server
            FD = socket(AF_UNIX, SOCK_STREAM, 0);
            if (FD == -1) {
                perror("p0: socket");
                exit(EXIT_FAILURE);
            }

            sockaddr_un server_addr{};
            memset(&server_addr, 0, sizeof(sockaddr_un));
            server_addr.sun_family = AF_UNIX;
            strncpy(server_addr.sun_path, "/tmp/your_socket_name.sock", sizeof(server_addr.sun_path) - 1);

            if (connect(FD, (struct sockaddr*)&server_addr, sizeof(sockaddr_un)) == -1) {
                perror("p0: connect");
                close(FD);
                FD = -1;
                attempt++;
            } else {
                std::cout << "p0: Successfully connected to server." << std::endl;
                break;
            }
        }

        if (FD == -1) {
            std::cerr << "p0: Failed to connect to the server after " << max_attempts << " attempts." << std::endl;
            exit(EXIT_FAILURE);
        }

        // Send a test request to the server to simulate activity
        std::string request_data = LoggerInstruction(src::terminal::InstructionType::LogInfo, FormatType::CTF, TransMode::Remote,
                                                     getpid(), "did a thing").Serialize();
        if (send(FD, request_data.c_str(), request_data.length(), 0) == -1) {
            std::cerr << "p0: Error in send(), errno: " << strerror(errno) << std::endl;
            close(FD);
            exit(EXIT_FAILURE);
        } else {
            std::cout << "p0: Sent test request to server." << std::endl;
        }

        // Wait for server response
        char buff[1024];
        ssize_t b_rec = recv(FD, buff, sizeof(buff) - 1, 0);
        if (b_rec < 0) {
            std::cerr << "p0: Error in recv(), errno: " << strerror(errno) << std::endl;
            close(FD);  // Close the socket on error
            exit(EXIT_FAILURE);
        } else if (b_rec == 0) {
            // Connection closed by the peer
            std::cerr << "p0: Connection closed by the server." << std::endl;
            close(FD);
            exit(EXIT_FAILURE);
        }

        // Wait for a moment to allow the server to process the termination
        std::this_thread::sleep_for(std::chrono::seconds(1));
        close(FD);

        std::cout << "p0: Connection and communication completed successfully." << std::endl;

        // Terminate the first child process
        exit(EXIT_SUCCESS);
    }

    std::cout << "Stopping TerminalUserInterface..." << std::endl;
    TerminalUserInterface::Stop();
    return 0;
}
