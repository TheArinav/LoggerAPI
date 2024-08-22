#ifndef LOGGEREXAMPLE_MAINLOGGER_HPP
#define LOGGEREXAMPLE_MAINLOGGER_HPP

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <any>
#include <functional>

#include "../IO/CustomFormatFileInterface.hpp"
#include "../IO/FileInterface.hpp"
#include "../IO/JsonFileInterface.hpp"
#include "./Enums.hpp"

#include "LogEntry.hpp"

using namespace std;

namespace src::logger_classes {

    class MainLogger {
    public:
        vector<LogEntry> Entries;
        string HomeDirectory;
        static void Setup();
        static any Exec(const function<any(MainLogger&)>& toDo);
        MainLogger(MainLogger const&) = delete;
        void operator=(MainLogger const&) = delete;
        shared_ptr<io::FileInterface> GetFileInterface(FileFormat format);

        shared_ptr<io::JSONFileInterface> cur_json=nullptr;
        shared_ptr<io::CustomFormatFileInterface> cur_ctf=nullptr;
        MainLogger();
        ~MainLogger();

        static shared_ptr<MainLogger> Instance;
    protected:
    private:

        static mutex m_Instance;
    };

} // logger_classes

#endif //LOGGEREXAMPLE_MAINLOGGER_HPP
