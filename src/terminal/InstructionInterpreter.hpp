#ifndef LOGGEREXAMPLE_INSTRUCTIONINTERPRETER_HPP
#define LOGGEREXAMPLE_INSTRUCTIONINTERPRETER_HPP

#include <string>
#include <sstream>
#include <memory>
#include "../networking/client/ServerConnection.hpp"
#include "../networking/transport/Constants.hpp"
#include "../networking/transport/Enums.hpp"
#include "../networking/transport/ClientResponse.hpp"
#include "../networking/transport/ServerRequest.hpp"
#include "../logger_classes/MainLogger.hpp"

using namespace std;

namespace src::terminal {
    enum class InstructionType {
        LogDebug,
        LogInfo,
        LogWarning,
        LogError,
        LogCritical,
        ReflectLog
    };
    enum class FormatType {
        Unspecified,
        JSON,
        CTF
    };
    enum class TransMode {
        Unspecified,
        Local,
        Remote
    };

    struct LoggerInstruction {
        InstructionType Type;
        FormatType FType;
        TransMode TMode;
        unsigned long PID;
        string Data;

        LoggerInstruction(InstructionType type, FormatType ftype, TransMode tmode, unsigned long pid, string data)
                : Type(type), FType(ftype), TMode(tmode), PID(pid), Data(move(data)) {}

        [[nodiscard]] string Serialize() const {
            stringstream ss;
            stringReplace(Data,"\n","\\n");
            ss << "["
               << static_cast<int>(Type) << " "
               << static_cast<int>(FType) << " "
               << static_cast<int>(TMode) << " "
               << PID << " "
               << "(" << Data << ")"
               << "]";
            return ss.str();
        }

        static LoggerInstruction Deserialize(const string &input) {
            stringstream ss(input);
            char buf;
            int i_type, i_ftype, i_tmode;
            unsigned long pid;

            // Read the opening bracket and the enum integer values
            ss >> buf;
            if (buf != '[') {
                throw runtime_error("Invalid format: Expected '[' at the beginning.");
            }

            ss >> i_type >> i_ftype >> i_tmode >> pid;
            if (ss.fail()) {
                throw runtime_error("Invalid format: Failed to parse integers.");
            }

            auto type = static_cast<InstructionType>(i_type);
            auto ftype = static_cast<FormatType>(i_ftype);
            auto tmode = static_cast<TransMode>(i_tmode);

            // Extract the remainder
            string remainder;
            getline(ss, remainder);

            // Find the start and end of the data
            size_t start = remainder.find('(');
            size_t end = remainder.find(')');
            if (start == string::npos || end == string::npos || start >= end) {
                throw runtime_error("Invalid format: Expected '(data)' format.");
            }

            // Extract the data content
            string data = remainder.substr(start + 1, end - start - 1);
            stringReplace(data,"\\n","\n");
            return {type, ftype, tmode, pid, data};
        }

    private:
        static std::string stringReplace(std::string str, const std::string &from, const std::string &to) {
            size_t start_pos = 0;
            while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length(); // Move past the last replacement
            }
            return str;
        }
    };

    class InstructionInterpreter {
    public:
        static shared_ptr<networking::client::ServerConnection> Connection;
        static void HandleInstruction(const LoggerInstruction &todo);

    private:
        static string getCanonicalName();

        static shared_ptr<src::logger_classes::MainLogger> GetLogger();
    };

} // terminal

#endif //LOGGEREXAMPLE_INSTRUCTIONINTERPRETER_HPP
