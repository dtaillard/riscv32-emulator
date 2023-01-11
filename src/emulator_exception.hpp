#ifndef __EMULATOR_EXCEPTION_HPP__ 
#define __EMULATOR_EXCEPTION_HPP__

#include <string>
#include <sstream>
#include <exception>
#include "instruction_type.hpp"

class EmulatorException: public std::exception {
    public:
        EmulatorException(const std::string& message);
        const std::string& what();
    private:
        std::string _message;
};

class DecoderException: public EmulatorException {
    public:
        DecoderException(const std::string& message, const InstructionFormat& fmt);
        const InstructionFormat& getInstructionFormat();
    private:
        const InstructionFormat fmt;
};

#endif /* __EMULATOR_EXCEPTION_HPP__ */
