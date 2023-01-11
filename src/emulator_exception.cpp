#include "emulator_exception.hpp"

EmulatorException::EmulatorException(const std::string& message): _message(message)  {}

const std::string& EmulatorException::what() {
        return _message;
}

DecoderException::DecoderException(const std::string& message, const InstructionFormat& fmt): EmulatorException(message), fmt(fmt) {}

const InstructionFormat& DecoderException::getInstructionFormat() {
        return fmt;
}
