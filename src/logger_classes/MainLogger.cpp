#include <iostream>
#include "MainLogger.hpp"

namespace src::logger_classes {
    shared_ptr<MainLogger> MainLogger::Instance= {};
    mutex MainLogger::m_Instance={};
    void MainLogger::Setup() {
        Instance = make_shared<MainLogger>();
    }

    any MainLogger::Exec(const function<any(MainLogger&)>& toDo) {
        if(!toDo) {
            perror("Nullptr in 'MainLogger.Exec'");
            exit(EXIT_FAILURE);
        }
        any res{};
        {
            lock_guard<mutex> guard_instance(m_Instance);
            res=toDo((MainLogger&)(*Instance));
        }
        return move(res);
    }

    MainLogger::MainLogger() {
    }

    MainLogger::~MainLogger() {

    }

    shared_ptr<io::FileInterface> MainLogger::GetFileInterface(FileFormat format) {
        switch (format) {
            case FileFormat::JSON:
                if (!cur_json) {
                    std::cerr << "JSON file interface not initialized!" << std::endl;
                    return nullptr;
                }
                return cur_json;
            case FileFormat::Custom:
                if (!cur_ctf) {
                    std::cerr << "Custom file interface not initialized!" << std::endl;
                    return nullptr;
                }
                return cur_ctf;
            default:
                std::cerr << "Unknown file format requested!" << std::endl;
                return nullptr;
        }
    }
} // logger_classes