#ifndef __DECODER_HPP__
#define __DECODER_HPP__

#include <string>
#include "instruction.hpp"

namespace RV32 {
    enum class Opcode;
    enum class InstructionType;

    Opcode decodeOpcode(Instruction instr);
    InstructionType decodeInstructionType(Instruction instr);
};

enum class RV32::InstructionType {
    LOAD, STORE, BRANCH, JUMP, AMO,
    OP_IMM, OP, SYSTEM, OP_UI, OP_FENCE
};

enum class RV32::Opcode {
    // RV32I Base //
    LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU,
    BGEU, LB, LH, LW, LBU, LHU, SB, SH, SW, ADDI, SLTI,
    SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, ADD,
    SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
    FENCE, ECALL, EBREAK, SRET, WFI, SFENCE_VMA,
    SINVAL_VMA, SFENCE_W_INVAL, SFENCE_INVAL_IR,

    // Zicsr Extension //
    CSRRW, CSRRS, CSRRC, CSRRWI, CSRRSI, CSRRCI,

    // Zifencei Extension //
    FENCE_I,

    // M Extension //
    MUL, MULH, MULHSU, MULHU, DIV, DIVU, REM, REMU,

    // A Extension //
    LR_W, SC_W, AMOSWAP_W, AMOADD_W, AMOXOR_W,
    AMOAND_W, AMOOR_W, AMOMIN_W, AMOMAX_W,
    AMOMINU_W, AMOMAXU_W
};


#endif /* __DECODER_HPP__ */
