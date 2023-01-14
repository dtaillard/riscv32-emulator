#ifndef __HART_HPP__
#define __HART_HPP__

#include "mem_map_manager.hpp"
#include "csr.hpp"
#include <chrono>

class HartRV32 {
    public:
        using PutCharCallback =  void (*)(char);

        // NOTE: getCharCallback() may NOT block
        using GetCharCallback =  char (*)(void);

        using ShutdownCallback = void (*)(void);

        HartRV32(uint32_t pc, MemoryMapManager &mem, uint32_t timebaseFreq, PutCharCallback, GetCharCallback, ShutdownCallback);
        virtual ~HartRV32() = default;

        class RegisterFile {
            public:
                static constexpr size_t NUM_GPR = 32;

                RegisterFile() { this->reset(); }

                // uint32_t& operator[](RegisterName index) {
                //     return gpr_array[static_cast<uint32_t>(index)];
                // }

                void reset();
                uint32_t getRegister(uint32_t index) const;
                void setRegister(uint32_t index, uint32_t value);
                
                private:
                    uint32_t gpr_array[NUM_GPR - 1];
        };

        void reset();
        void stepInstruction();

        uint32_t pc;

        RegisterFile& getRegisterFile() { return gpr; }
        CSRFile& getCSRFile() { return csr; }
    private:
        uint32_t tempPc;
        RegisterFile gpr;
        CSRFile csr;

        MemoryMapManager &mem;

        const uint64_t SECONDS_TO_NANSECONDS = 1000000000;
        std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
        uint64_t timebasePeriod;
        uint64_t timeCompare = 0;

        PutCharCallback putCharCallback;
        GetCharCallback getCharCallback;
        ShutdownCallback shutdownCallback;

        bool supervisorMode = true;
        bool didExceptionOccur = false;
        bool shouldIncrementPC = false;
        bool reservationSetValid = false;

        enum class ExceptionCode: uint32_t {
            INSTR_ILLEGAL_EXC               = 2,
            BREAKPOINT                      = 3,

            INSTR_MISALIGNED_EXC            = 0,
            LOAD_MISALIGNED_EXC             = 4,
            STR_AMO_MISALIGNED_EXC          = 6,

            LOAD_ACCESS_FAULT_EXC           = 5,
            STR_AMO_ACCESS_FAULT_EXC        = 7,
            INSTR_ACCESS_FAULT_EXC          = 1,

            U_ECALL_EXC                     = 8,
            S_ECALL_EXC                     = 9,

            INSTR_PAGE_FAULT_EXC            = 12,
            LOAD_PAGE_FAULT_EXC             = 13,
            STR_AMO_PAGE_FAULT_EXC          = 15
        };

        enum class MemoryAccessType: uint32_t {
            READ, WRITE, EXECUTE
        };

        void handleInterrupt();
        void incrementTimer();
        void handleException(ExceptionCode code, uint32_t stval);
        bool translateAddress(uint32_t &addr, MemoryAccessType accessType);
};



#endif /* __HART_HPP__ */

