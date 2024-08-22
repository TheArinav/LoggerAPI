#ifndef LOGGEREXAMPLE_TERMINALUSERINTERFACE_HPP
#define LOGGEREXAMPLE_TERMINALUSERINTERFACE_HPP

#include <iostream>
#include <thread>
#include <atomic>

#include "../logger_classes/MainLogger.hpp"
#include "./InstructionInterpreter.hpp"

using namespace src::logger_classes;
using namespace std;

namespace src::terminal {

        class TerminalUserInterface {
        public:
            static void Start();
            static void Stop();
        private:
            static atomic<int> FD;
            static atomic<int> EpollFD;
            static atomic<bool> f_Stop;
            static vector<LoggerInstruction> Instructions;
        };

    } // terminal

#endif //LOGGEREXAMPLE_TERMINALUSERINTERFACE_HPP
