#ifndef LOGGEREXAMPLE_SERVERREQUEST_HPP
#define LOGGEREXAMPLE_SERVERREQUEST_HPP

#include <string>
#include <sstream>
#include "../../logger_classes/Enums.hpp"
#include "../../logger_classes/LogEntry.hpp"

using namespace std;

namespace src::networking::transport {
    class ServerRequest {
    public:
        string Data;
        logger_classes::FileFormat Format;
        logger_classes::Severity Severity;
        unsigned long Origin;
        explicit ServerRequest(logger_classes::FileFormat format, logger_classes::Severity severity, unsigned long origin, string data)
                : Data(move(data)), Format(format), Severity(severity), Origin(origin) {}

        [[nodiscard]] string Serialize() const {
            stringstream ss{};
            string data = stringReplace(Data, "\n", "\\n");
            ss << static_cast<int>(Format) << " "
               << static_cast<int>(Severity) << " "
               << Origin << " "
               << data
               << endl;
            return ss.str();
        }

        static ServerRequest Deserialize(const string& toDes) {
            stringstream ss{toDes};
            int i_format, i_severity;
            unsigned long origin;
            string data;
            ss >> i_format >> i_severity >> origin;
            getline(ss, data);
            data = stringReplace(data, "\\n", "\n");
            return ServerRequest(static_cast<logger_classes::FileFormat>(i_format),
                                 static_cast<logger_classes::Severity>(i_severity),
                                 origin, data);
        }

    private:
        static std::string stringReplace(std::string str, const std::string& from, const std::string& to) {
            size_t start_pos = 0;
            while((start_pos = str.find(from, start_pos)) != std::string::npos) {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length(); // Move past the last replacement
            }
            return str;
        }
    };
} // namespace src::networking::transport


#endif //LOGGEREXAMPLE_SERVERREQUEST_HPP
