#ifndef __GPR_HPP__
#define __GPR_HPP__

#include <cstdint>

#define NUM_GPR 32

class GPR {
    public:
        /*
         * The ABI name of the registers
         */
        enum {
            ZERO = 0,
            RA,
            SP,
            GP,
            TP,
            T0,
            T1,
            T2,
            S0,
            S1,
            A0,
            A1,
            A2,
            A3,
            A4,
            A5,
            A6,
            A7,
            S2,
            S3,
            S4,
            S5,
            S6,
            S7,
            S8,
            S9,
            S10,
            S11,
            T3,
            T4,
            T5,
            T6
        };

        void reset();
        uint32_t getRegister(uint32_t index) const;
        void setRegister(uint32_t index, uint32_t value);
    private:
        uint32_t _gpr[NUM_GPR];
};

#endif /* __GPR_HPP__ */

