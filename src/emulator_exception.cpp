#include "emulator_exception.hpp"

EmulatorException::EmulatorException(const std::string& message): _message(message)  {}

const std::string& EmulatorException::what() {
        return _message;
}
