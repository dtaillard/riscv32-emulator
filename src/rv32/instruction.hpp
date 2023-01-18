#ifndef __INSTRUCTION_HPP__
#define __INSTRUCTION_HPP__

#include <cstdint>
#include "bit_field.hpp"

#define SIGN_EXTEND(n, b) ((-((n) >> ((b) - 1)) << (b)) | (n))

namespace RV32 {
    union Instruction {
        uint32_t bits;

        U32BitField<0, 6> opcode;

        union {
            U32BitField<7, 11>   rd; 
            U32BitField<12, 14>  funct3; 
            U32BitField<15, 19>  rs1; 
            U32BitField<20, 24>  rs2; 
            U32BitField<25, 31>  funct7; 
        } r;

        union {
            U32BitField<7, 11>   rd; 
            U32BitField<12, 14>  funct3; 
            U32BitField<15, 19>  rs1; 
            U32BitField<20, 31>  imm_11_0;

            uint32_t immediateValue() const {
                return SIGN_EXTEND(imm_11_0, 12);
            }
        } i;

        union {
            U32BitField<12, 14>  funct3; 
            U32BitField<15, 19>  rs1; 
            U32BitField<20, 24>  rs2; 
            U32BitField<25, 31>  imm_11_5; 
            U32BitField<7, 11>   imm_4_0;

            uint32_t immediateValue() const {
                return SIGN_EXTEND((imm_11_5 << 5) | imm_4_0, 12);
            }
        } s;

        union {
            U32BitField<12, 14>  funct3;
            U32BitField<15, 19>  rs1;
            U32BitField<20, 24>  rs2;
            U32BitField<25, 30>  imm_10_5;
            U32BitField<31, 31>  imm_12;
            U32BitField<8, 11>   imm_4_1;
            U32BitField<7, 7>    imm_11;

            uint32_t immediateValue() const {
                return SIGN_EXTEND((imm_12 << 12) | (imm_11 << 11) | (imm_10_5 << 5) | (imm_4_1 << 1), 13);
            }
        } b;

        union {
            U32BitField<7, 11>   rd; 
            U32BitField<12, 31>  imm_31_12;

            uint32_t immediateValue() const {
                return (imm_31_12 << 12);
            } 
        } u;

        union {
            U32BitField<7, 11>   rd; 
            U32BitField<31, 31>  imm_20; 
            U32BitField<21, 30>  imm_10_1; 
            U32BitField<20, 20>  imm_11; 
            U32BitField<12, 19>  imm_19_12;

            uint32_t immediateValue() const {
                return SIGN_EXTEND((imm_20 << 20) | (imm_19_12 << 12) | (imm_11 << 11) | (imm_10_1 << 1), 21);
            }
        } j;
    };
};

#endif /* __INSTRUCTION_HPP__ */