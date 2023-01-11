#include "decoder.hpp"
#include "emulator_exception.hpp"

using Decoder::Type;
using Decoder::Opcode;

Type Decoder::getType(uint32_t instr) {
    InstructionFormat fmt = instr;
    switch(fmt.opcode) {
        case 0b0000011: 
            return Type::LOAD;
        case 0b0100011:
            return Type::STORE;
        case 0b1100011:
            return Type::BRANCH;
        case 0b1100111:
        case 0b1101111:
            return Type::JUMP;
        case 0b0101111:
            return Type::AMO;
        case 0b0010011:
            return Type::OP_IMM;
        case 0b0110011:
            return Type::OP;
        case 0b1110011:
            return Type::SYSTEM;
        case 0b0110111:
        case 0b0010111:
            return Type::OP_UI;
        case 0b0001111:
            return Type::OP_FENCE;
        default:
            throw DecoderException("Unknown instruction type", fmt);
    }
}

Opcode Decoder::getOpcode(uint32_t instr) {
    switch(getType(instr)) {
        case Type::LOAD: {
            InstructionIType fmt = instr;
            switch(fmt.funct3) {
                case 0b000:
                    return Opcode::LB;
                case 0b001:
                    return Opcode::LH;
                case 0b010:
                    return Opcode::LW;
                case 0b100:
                    return Opcode::LBU;
                case 0b101:
                    return Opcode::LHU;
                default:
                    throw DecoderException("Unknown LOAD instruction", fmt);
            }
        }
        case Type::STORE: {
            InstructionSType fmt = instr;
            switch(fmt.funct3) {
                case 0b000:
                    return Opcode::SB;
                case 0b001:
                    return Opcode::SH;
                case 0b010:
                    return Opcode::SW;
                default:
                    throw DecoderException("Unknown STORE instruction", fmt);
            }
        }
        case Type::BRANCH: {
            InstructionBType fmt = instr;
            switch(fmt.funct3) {
                case 0b000:
                    return Opcode::BEQ;
                case 0b001:
                    return Opcode::BNE;
                case 0b100:
                    return Opcode::BLT;
                case 0b101:
                    return Opcode::BGE;
                case 0b110:
                    return Opcode::BLTU;
                case 0b111:
                    return Opcode::BGEU;
                default:
                    throw DecoderException("Unknown BRANCH instruction", fmt);
            }
        }
        case Type::JUMP: {
            InstructionFormat fmt = instr;
            switch(fmt.opcode) {
                case 0b1100111:
                    return Opcode::JALR;
                case 0b1101111:
                    return Opcode::JAL;
                default:
                    throw DecoderException("Unknown JUMP instruction", fmt);
            } 
        }
        case Type::AMO: {
            InstructionRType fmt = instr;
            // Only upper 5 bits of funct7 encode the opcode:
            switch(fmt.funct7 >> 2) {
                case 0b00010:
                    return Opcode::LR_W;
                case 0b00011:
                    return Opcode::SC_W;
                case 0b00001:
                    return Opcode::AMOSWAP_W;
                case 0b00000:
                    return Opcode::AMOADD_W;
                case 0b00100:
                    return Opcode::AMOXOR_W;
                case 0b01100:
                    return Opcode::AMOAND_W;
                case 0b01000:
                    return Opcode::AMOOR_W;
                case 0b10000:
                    return Opcode::AMOMIN_W;
                case 0b10100:
                    return Opcode::AMOMAX_W;
                case 0b11000:
                    return Opcode::AMOMINU_W;
                case 0b11100:
                    return Opcode::AMOMAXU_W;
                default:
                    throw DecoderException("Unknown AMO instruction", fmt);
            }
        }
        case Type::OP_IMM: {
             InstructionRType fmt = instr;
             switch(fmt.funct3) {
                 case 0b000:
                     return Opcode::ADDI;
                 case 0b010:
                     return Opcode::SLTI;
                 case 0b011:
                     return Opcode::SLTIU;
                 case 0b100:
                     return Opcode::XORI;
                 case 0b110:
                     return Opcode::ORI;
                 case 0b111:
                     return Opcode::ANDI;
                 case 0b001:
                     return Opcode::SLLI;
                 case 0b101:
                     if(fmt.funct7 == 0b0000000) {
                         return Opcode::SRLI;
                     } else {
                         return Opcode::SRAI;
                     }
                default:
                    throw DecoderException("Unknown OP_IMM instruction", fmt);
             }
        }
        case Type::OP: {
            InstructionRType fmt = instr;
            if((fmt.funct7 & 0b0000001) == 0) {
                switch(fmt.funct3) {
                    case 0b000:
                        if(fmt.funct7 == 0b0000000) {
                            return Opcode::ADD;
                        } else {
                            return Opcode::SUB;
                        }
                    case 0b001:
                        return Opcode::SLL;
                    case 0b010:
                        return Opcode::SLT;
                    case 0b011:
                        return Opcode::SLTU;
                    case 0b100:
                        return Opcode::XOR;
                    case 0b101:
                        if(fmt.funct7 == 0b0000000) {
                            return Opcode::SRL;
                        } else {
                            return Opcode::SRA;
                        }
                    case 0b110:
                        return Opcode::OR;
                    case 0b111:
                        return Opcode::AND;
                    default:
                        throw DecoderException("Unknown OP instruction", fmt);
                }
            } else {
                switch(fmt.funct3) {
                    case 0b000:
                        return Opcode::MUL;
                    case 0b001:
                        return Opcode::MULH;
                    case 0b010:
                        return Opcode::MULHSU;
                    case 0b011:
                        return Opcode::MULHU;
                    case 0b100:
                        return Opcode::DIV;
                    case 0b101:
                        return Opcode::DIVU;
                    case 0b110:
                        return Opcode::REM;
                    case 0b111:
                        return Opcode::REMU;
                    default:
                        throw DecoderException("Unknown M-extension instruction", fmt);
                }
            }
        }
        case Type::SYSTEM: {
            InstructionIType fmt = instr;
            InstructionRType fmtR = instr;

            switch(fmt.funct3) {
                case 0b000:
                    switch(fmt.getImmediateValue()) {
                        case 0x000:
                            return Opcode::ECALL;
                        case 0x001:
                            return Opcode::EBREAK;
                        case 0x102:
                            return Opcode::SRET;
                        case 0x105:
                            return Opcode::WFI;
                        default:
                            switch(fmtR.funct7) {
                                case 0b0001001:
                                    return Opcode::SFENCE_VMA;
                                case 0b0001011:
                                    return Opcode::SINVAL_VMA;
                                case 0b0001100:
                                    if(fmtR.rs2 == 0b00000) {
                                        return Opcode::SFENCE_W_INVAL;
                                    } else {
                                        return Opcode::SFENCE_INVAL_IR;
                                    }
                            }
                    }
                case 0b001:
                    return Opcode::CSRRW;
                case 0b010:
                    return Opcode::CSRRS;
                case 0b011:
                    return Opcode::CSRRC;
                case 0b101:
                    return Opcode::CSRRWI;
                case 0b110:
                    return Opcode::CSRRSI;
                case 0b111:
                    return Opcode::CSRRCI;
                default:
                    throw DecoderException("Unknown SYSTEM instruction", fmt);
            }
        }
        case Type::OP_UI: {
            InstructionFormat fmt = instr;
            switch(fmt.opcode) {
                case 0b0110111:
                    return Opcode::LUI;
                case 0b0010111:
                    return Opcode::AUIPC;
                default:
                    throw DecoderException("Unknown OP_UI instruction", fmt);
            }
        }
        case Type::OP_FENCE: {
            InstructionIType fmt = instr;
            switch(fmt.funct3) {
                case 0b000:
                    return Opcode::FENCE;
                case 0b001:
                    return Opcode::FENCE_I;
                default:
                    throw DecoderException("Unknown OP_FENCE instruction", fmt);
            }
        }
    }
}
