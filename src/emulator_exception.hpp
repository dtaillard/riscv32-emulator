#ifndef __EMULATOR_EXCEPTION_HPP__ 
#define __EMULATOR_EXCEPTION_HPP__

#include <string>
#include <sstream>
#include <exception>
#include "instruction.hpp"

class EmulatorException: public std::exception {
    public:
        EmulatorException(const std::string& message);
        const std::string& what();
    private:
        std::string message;
};

namespace RV32 {
    class DecoderException: public EmulatorException {
        public:
            DecoderException(const std::string& message, Instruction instr);
            const Instruction& getInstruction();
        private:
            const Instruction instr;
    };
};

#endif /* __EMULATOR_EXCEPTION_HPP__ */
