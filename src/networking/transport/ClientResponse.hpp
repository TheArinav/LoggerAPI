#ifndef LOGGEREXAMPLE_CLIENTRESPONSE_HPP
#define LOGGEREXAMPLE_CLIENTRESPONSE_HPP

#include <string>
#include <sstream>

using namespace std;

namespace src::networking::transport {
    class ClientResponse {
    public:
        bool Succeeded;
        string Data;
        ClientResponse(bool succeeded, string data): Succeeded(succeeded), Data(move(data)){}
        [[nodiscard]] string Serialize() const{
            stringstream ss{};
            ss << static_cast<int>(Succeeded)
               << " "
               << Data
               << endl;
            return ss.str();
        }
        static ClientResponse Deserialize(const string& toDes){
            stringstream ss{toDes};
            int suc;
            string data;
            ss >> suc;
            getline(ss, data);
            data = data.substr(1, data.size() - 1);
            return {static_cast<bool>(suc), data};
        }
    };
} // namespace src::networking::transport


#endif //LOGGEREXAMPLE_CLIENTRESPONSE_HPP
