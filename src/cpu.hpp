#ifndef __CPU_HPP__
#define __CPU_HPP__

#include "memory.hpp"
#include "gpr.hpp"

class RV32CPU {
    public:
        RV32CPU(uint32_t pc, const LEMemory &memory);
        virtual ~RV32CPU() = default;

        void reset();
        void stepInstruction();

        uint32_t getPC();
        void setPC(uint32_t pc);

        GPR getRegisters();
    private:
        GPR _gpr;
        uint32_t _pc;
        LEMemory _memory;
};

#endif /* __CPU_HPP__ */

