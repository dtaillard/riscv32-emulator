#include "emulator_exception.hpp"

EmulatorException::EmulatorException(const std::string& message): message(message)  {}

const std::string& EmulatorException::what() {
        return message;
}

RV32::DecoderException::DecoderException(const std::string& message, RV32::Instruction instr): EmulatorException(message), instr(instr) {}

const RV32::Instruction& RV32::DecoderException::getInstruction() {
        return instr;
}
