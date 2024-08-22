#ifndef LOGGEREXAMPLE_ENUMS_HPP
#define LOGGEREXAMPLE_ENUMS_HPP
namespace src::logger_classes{
    enum class FileFormat{
        JSON,
        Custom
    };
    enum class TransportMode{
        NONE,
        Local,
        Remote
    };
}
#endif //LOGGEREXAMPLE_ENUMS_HPP
