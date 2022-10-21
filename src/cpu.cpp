#include "cpu.hpp"
#include "emulator_exception.hpp"

#define OPCODE_MASK     0x0000007F

// R-Type instruction masks
#define FUNCT7_MASK      0xFE000000
#define RS2_MASK         0x01F00000
#define RS1_MASK         0x00F10000
#define FUNCT3_MASK      0x00007000
#define RD_MASK          0x00001F00
#define IMM_11_0         0xFFF00000

#define SIGN_EXTEND(n, b) ((-(n >> (b - 1)) << b) | n)

RV32CPU::RV32CPU(uint32_t pc, const LEMemory &memory) : _pc(pc), _memory(memory) {
    this->reset();
}

void RV32CPU::stepInstruction() {
    const uint32_t instrBits = _memory.readWord(_pc);
    _pc += 4;

    switch(instr & OPCODE_MASK) {
        // LOAD
        case 0x03: {
                // C bit fields with union
                // gotta shift these if not doing this
                const uint32_t rs1     = instrBits & RS1_MASK;
                const uint32_t rd      = instrBits & RD_MASK;
                const uint32_t funct3  = instrBits & FUNCT3_MASK;
                const uint32_t offset  = instrBits & IMM_11_0;

                const uint32_t effectiveAddr = _gpr.getRegister(rs1) + SIGN_EXTEND(offset, 12);

                switch(funct3) {
                    case 0x0: // LB
                        _gpr.setRegister(rd, SIGN_EXTEND(_memory.readByte(effectiveAddr), 8));
                        break;
                    case 0x1: // LH
                        _gpr.setRegister(rd, SIGN_EXTEND(_memory.readHalfword(effectiveAddr), 16));
                        break;
                    case 0x2: // LW
                        _gpr.setRegister(rd, _memory.readWord(effectiveAddr));
                        break;
                    case 0x4: // LBU
                        _gpr.setRegister(rd, _memory.readByte(effectiveAddr));
                        break;
                    case 0x4: // LHU
                        _gpr.setRegister(rd, _memory.readHalfword(effectiveAddr));
                        break;
                }
            }
            break;
        // STORE
        case 0x23: {
                const uint32_t rs1     = instrBits & RS1_MASK;
                const uint32_t rs2     = instrBits & RS2_MASK;
                const uint32_t funct3  = instrBits & FUNCT3_MASK;
                const uint32_t offset = ((instrBits & IMM_4_0) >> 7) | ((instrBirs & IMM_11_5) >> 20);

                const uint32_t effectiveAddr = _gpr.getRegister(rs1) + SIGN_EXTEND(offset, 12);

                switch(funct3) {
                    case 0x0: // SB
                        _memory.writeByte(effectiveAddr, _gpr.getRegister(rs2) & 0xFF);
                        break;
                    case 0x1: // SH
                        _memory.writeHalfword(effectiveAddr, _gpr.getRegister(rs2) & 0xFFFF);
                        break;
                    case 0x2: // SW
                        _memory.writeHalfword(effectiveAddr, _gpr.getRegister(rs2));
                        break;
                }
            }
            break;
        default:
            throw EmulatorException("Illegal instruction");
    }
}

void RV32CPU::reset() {
    _gpr.reset();
}
