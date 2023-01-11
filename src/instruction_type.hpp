#ifndef __INSTRUCTION_TYPE_HPP__
#define __INSTRUCTION_TYPE_HPP__

#include <cstdint>
#include "bit_field.hpp"

#define SIGN_EXTEND(n, b) ((-((n) >> ((b) - 1)) << (b)) | (n))

struct InstructionFormat {
    InstructionFormat(uint32_t value): value(value) {}
    // InstructionFormat(const InstructionFormat& other) = default;

    virtual operator uint32_t() const {
        return value;
    }

    uint32_t value;
    const U32BitField<0, 6>    opcode  {value}; 
};

struct InstructionRType: public InstructionFormat {
    InstructionRType(uint32_t value): InstructionFormat(value) {}

    const U32BitField<7, 11>   rd      {value}; 
    const U32BitField<12, 14>  funct3  {value}; 
    const U32BitField<15, 19>  rs1     {value}; 
    const U32BitField<20, 24>  rs2     {value}; 
    const U32BitField<25, 31>  funct7  {value}; 
};

struct InstructionIType: public InstructionFormat {
    InstructionIType(uint32_t value): InstructionFormat(value) {}

    uint32_t getImmediateValue() const {
        return SIGN_EXTEND(imm_11_0, 12);
    }

    // CSR instructions encode function in immediate 11:0
    // Do not sign extend this
    uint32_t getCSRField() const {
        return imm_11_0;
    }

    const U32BitField<7, 11>   rd       {value}; 
    const U32BitField<12, 14>  funct3   {value}; 
    const U32BitField<15, 19>  rs1      {value}; 

    const U32BitField<20, 31>  imm_11_0 {value}; 
};

struct InstructionSType: public InstructionFormat {
    InstructionSType(uint32_t value): InstructionFormat(value) {}

    uint32_t getImmediateValue() const {
        return SIGN_EXTEND((imm_11_5 << 5) | imm_4_0, 12);
    }

    const U32BitField<12, 14>  funct3   {value}; 
    const U32BitField<15, 19>  rs1      {value}; 
    const U32BitField<20, 24>  rs2      {value}; 

    const U32BitField<25, 31> imm_11_5  {value}; 
    const U32BitField<7, 11>  imm_4_0   {value}; 
};

struct InstructionBType: public InstructionFormat {
    InstructionBType(uint32_t value): InstructionFormat(value) {}

    uint32_t getImmediateValue() const {
        return SIGN_EXTEND((imm_12 << 12) | (imm_11 << 11) | (imm_10_5 << 5) | (imm_4_1 << 1), 13);
    }

    const U32BitField<12, 14>  funct3   {value}; 
    const U32BitField<15, 19>  rs1      {value}; 
    const U32BitField<20, 24>  rs2      {value}; 

    const U32BitField<25, 30> imm_10_5  {value}; 
    const U32BitField<31, 31> imm_12    {value}; 
    const U32BitField<8, 11>  imm_4_1   {value}; 
    const U32BitField<7, 7>   imm_11    {value}; 
};
 
struct InstructionUType: public InstructionFormat {
    InstructionUType(uint32_t value): InstructionFormat(value) {}

    uint32_t getImmediateValue() const {
        return (imm_31_12 << 12);
    }

    U32BitField<7, 11>   rd        {value}; 

    U32BitField<12, 31> imm_31_12  {value}; 
};

struct InstructionJType: public InstructionFormat {
    InstructionJType(uint32_t value): InstructionFormat(value) {}

    uint32_t getImmediateValue() const {
        return SIGN_EXTEND((imm_20 << 20) | (imm_19_12 << 12) | (imm_11 << 11) | (imm_10_1 << 1), 21);
    }

    const U32BitField<7, 11>   rd        {value}; 

    const U32BitField<31, 31> imm_20     {value}; 
    const U32BitField<21, 30> imm_10_1   {value}; 
    const U32BitField<20, 20> imm_11     {value}; 
    const U32BitField<12, 19> imm_19_12  {value}; 
};

#endif /* __INSTRUCTION_TYPE_HPP__ */
