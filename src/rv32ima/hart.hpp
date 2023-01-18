#ifndef __HART_HPP__
#define __HART_HPP__

#include "mem_map_manager.hpp"
#include "csr.hpp"
#include <chrono>

namespace RV32 {
    union Registers {
        static constexpr size_t NUM_GPR = 32;

        uint32_t r[NUM_GPR];

        struct {
            uint32_t zero;
            uint32_t ra;
            uint32_t sp;
            uint32_t gp;
            uint32_t tp;
            uint32_t t0;
            uint32_t t1;
            uint32_t t2;
            uint32_t s0;
            uint32_t s1;
            uint32_t a0;
            uint32_t a1;
            uint32_t a2;
            uint32_t a3;
            uint32_t a4;
            uint32_t a5;
            uint32_t a6;
            uint32_t a7;
            uint32_t s2;
            uint32_t s3;
            uint32_t s4;
            uint32_t s5;
            uint32_t s6;
            uint32_t s7;
            uint32_t s8;
            uint32_t s9;
            uint32_t s10;
            uint32_t s11;
            uint32_t t3;
            uint32_t t4;
            uint32_t t5;
            uint32_t t6;
        };

        void reset() {
            for(auto& reg : r) {
                reg = 0;
            }
        }
    };

    struct HartConfig {
        using ShutdownCallback = void (*)(void);
        using PutCharCallback =  void (*)(char);
        using GetCharCallback =  char (*)(void);

        uint32_t timebaseFreq;
        ShutdownCallback shutdownCallback;
        PutCharCallback putCharCallback;
        GetCharCallback getCharCallback;
    };

    class Hart {
        public:
            Hart(uint32_t pc, MemoryMapManager &mem, const HartConfig& config);
            virtual ~Hart() = default;

            void reset() { gpr.reset(); }
            void stepInstruction();

            void setPC(uint32_t addr) { pc = addr; }
            uint32_t getPC() const { return pc; }

            Registers& getRegisters() { return gpr; }
            CSRs& getCSRs() { return csr; }
            MemoryMapManager& getMemoryMapManager() { return mem; }
        private:
            const uint64_t SECONDS_TO_NANSECONDS = 1000000000;

            enum class MemoryAccessType: uint32_t {
                READ, WRITE, EXECUTE
            };

            enum class ExceptionCode: uint32_t {
                INSTR_ILLEGAL_EXC         = 2,
                BREAKPOINT                = 3,

                INSTR_MISALIGNED_EXC      = 0,
                LOAD_MISALIGNED_EXC       = 4,
                STR_AMO_MISALIGNED_EXC    = 6,

                LOAD_ACCESS_FAULT_EXC     = 5,
                STR_AMO_ACCESS_FAULT_EXC  = 7,
                INSTR_ACCESS_FAULT_EXC    = 1,

                U_ECALL_EXC               = 8,
                S_ECALL_EXC               = 9,

                INSTR_PAGE_FAULT_EXC      = 12,
                LOAD_PAGE_FAULT_EXC       = 13,
                STR_AMO_PAGE_FAULT_EXC    = 15
            };
            
            const HartConfig hartConfig;
            MemoryMapManager &mem;
            Registers gpr;
            CSRs csr;
            uint32_t pc;

            std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;

            const uint64_t timebasePeriod;
            uint64_t timeCompare = 0;

            bool supervisorMode = true;
            bool shouldIncrementPC = false;
            bool reservationSetValid = false;

            void handleInterrupts();
            void incrementCounters();

            void setRegister(uint32_t index, uint32_t value);
            uint32_t getRegister(uint32_t index) const;

            void handleException(ExceptionCode code, uint32_t stval);
            bool translateAddress(uint32_t &addr, MemoryAccessType accessType);
    };
};

#endif /* __HART_HPP__ */

