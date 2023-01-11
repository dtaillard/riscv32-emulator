#include "hart.hpp"
#include "emulator_exception.hpp"
#include "instruction_type.hpp"
#include "decoder.hpp"
#include "vmem.hpp"
#include <iostream>

HartRV32::HartRV32(uint32_t pc, MemoryMapManager &mem, uint32_t timebaseFreq, PutCharCallback pcc, GetCharCallback gcc, ShutdownCallback sdc)
    : pc(pc), mem(mem), putCharCallback(pcc), getCharCallback(gcc), shutdownCallback(sdc), lastTime(std::chrono::system_clock::now()) {
    this->timebasePeriod = SECONDS_TO_NANSECONDS * (1.0 / timebaseFreq);
    this->reset();
}

void HartRV32::stepInstruction() {
    using Decoder::Opcode;
    using Decoder::Type;

    if((pc & 0b11) != 0) {
        handleException(ExceptionCode::INSTR_MISALIGNED_EXC, pc);
    }

    handleInterrupt();

    uint32_t pcPhysicalAddr = pc;
    if(!translateAddress(pcPhysicalAddr, MemoryAccessType::EXECUTE)) {
        handleException(ExceptionCode::INSTR_PAGE_FAULT_EXC, pc);
        pcPhysicalAddr = pc;
        if(!translateAddress(pcPhysicalAddr, MemoryAccessType::EXECUTE)) {
            throw EmulatorException("Page fault while translating exception handler address");
        }
    }

    uint32_t instrBits = mem.readWord(pcPhysicalAddr);
    shouldIncrementPC = true;

    switch(Decoder::getType(instrBits)) {
        case Type::LOAD: {
                InstructionIType fmtI = instrBits;
                uint32_t effectiveAddr = gpr.getRegister(fmtI.rs1) + fmtI.getImmediateValue();

                if(!translateAddress(effectiveAddr, MemoryAccessType::READ)) {
                    handleException(ExceptionCode::LOAD_PAGE_FAULT_EXC, effectiveAddr);
                    break;
                }

                switch(Decoder::getOpcode(instrBits)) {
                    case Opcode::LB:
                        gpr.setRegister(fmtI.rd, SIGN_EXTEND(mem.readByte(effectiveAddr), 8));
                        break;
                    case Opcode::LH:
                        if((effectiveAddr & 0b1) != 0) {
                            handleException(ExceptionCode::LOAD_MISALIGNED_EXC, effectiveAddr);
                            break;
                        }
                        gpr.setRegister(fmtI.rd, SIGN_EXTEND(mem.readHalfword(effectiveAddr), 16));
                        break;
                    case Opcode::LW:
                        if((effectiveAddr & 0b11) != 0) {
                            handleException(ExceptionCode::LOAD_MISALIGNED_EXC, effectiveAddr);
                            break;
                        }
                        gpr.setRegister(fmtI.rd, mem.readWord(effectiveAddr));
                        break;
                    case Opcode::LBU:
                        gpr.setRegister(fmtI.rd, mem.readByte(effectiveAddr));
                        break;
                    case Opcode::LHU:
                        if((effectiveAddr & 0b1) != 0) {
                            handleException(ExceptionCode::LOAD_MISALIGNED_EXC, effectiveAddr);
                            break;
                        }
                        gpr.setRegister(fmtI.rd, mem.readHalfword(effectiveAddr));
                        break;
                }
            }
            break;
        case Type::STORE: {
                InstructionSType fmtS = instrBits;
                uint32_t effectiveAddr = gpr.getRegister(fmtS.rs1) + fmtS.getImmediateValue();

                if(!translateAddress(effectiveAddr, MemoryAccessType::WRITE)) {
                    handleException(ExceptionCode::STR_AMO_PAGE_FAULT_EXC, effectiveAddr);
                    break;
                }

                switch(Decoder::getOpcode(instrBits)) {
                    case Opcode::SB:
                        mem.writeByte(effectiveAddr, gpr.getRegister(fmtS.rs2) & 0xFF);
                        break;
                    case Opcode::SH:
                        if((effectiveAddr & 0b1) != 0) {
                            handleException(ExceptionCode::STR_AMO_MISALIGNED_EXC, effectiveAddr);
                            break;
                        }
                        mem.writeHalfword(effectiveAddr, gpr.getRegister(fmtS.rs2) & 0xFFFF);
                        break;
                    case Opcode::SW:
                        if((effectiveAddr & 0b11) != 0) {
                            handleException(ExceptionCode::STR_AMO_MISALIGNED_EXC, effectiveAddr);
                            break;
                        }
                        mem.writeWord(effectiveAddr, gpr.getRegister(fmtS.rs2));
                        break;
                }
            }
            break;
        case Type::BRANCH: {
            InstructionBType fmtB = instrBits;

            uint32_t rs1Value = gpr.getRegister(fmtB.rs1);
            uint32_t rs2Value = gpr.getRegister(fmtB.rs2);
            
            int32_t rs1ValueSigned = rs1Value;
            int32_t rs2ValueSigned = rs2Value;

            uint32_t effectiveAddr = pc + fmtB.getImmediateValue();

            switch(Decoder::getOpcode(instrBits)) {
                case Opcode::BEQ:
                    if(rs1Value == rs2Value) {
                        shouldIncrementPC = false;
                        pc = effectiveAddr;
                    }
                break;
                case Opcode::BNE:
                    if(rs1Value != rs2Value) {
                        shouldIncrementPC = false;
                        pc = effectiveAddr;
                    }
                break;
                case Opcode::BLT:
                    if(rs1ValueSigned < rs2ValueSigned) {
                        shouldIncrementPC = false;
                        pc = effectiveAddr;
                    }
                break;
                case Opcode::BGE:
                    if(rs1ValueSigned >= rs2ValueSigned) {
                        shouldIncrementPC = false;
                        pc = effectiveAddr;
                    }
                break;
                case Opcode::BLTU:
                    if(rs1Value < rs2Value) {
                        shouldIncrementPC = false;
                        pc = effectiveAddr;
                    }
                break;
                case Opcode::BGEU:
                    if(rs1Value >= rs2Value) {
                        shouldIncrementPC = false;
                        pc = effectiveAddr;
                    }
                break;
            }
            break;
        }
        case Type::JUMP:
            shouldIncrementPC = false;
            switch(Decoder::getOpcode(instrBits)) {
                    case Opcode::JAL: {
                        InstructionJType fmtJ = instrBits;
                        gpr.setRegister(fmtJ.rd, pc + 4);
                        uint32_t effectiveAddr = pc + fmtJ.getImmediateValue();
                        pc = effectiveAddr;
                        break;
                    }
                    case Opcode::JALR: {
                        InstructionIType fmtI = instrBits;
                        uint32_t effectiveAddr = gpr.getRegister(fmtI.rs1) + fmtI.getImmediateValue();
                        gpr.setRegister(fmtI.rd, pc + 4);
                        pc = effectiveAddr & 0xFFFFFFFE; // clear least significant bit
                        break;
                    }
            }
            break;
        case Type::AMO: {
            InstructionRType fmtR = instrBits;

            uint32_t addr = gpr.getRegister(fmtR.rs1);

            Opcode opcode = Decoder::getOpcode(instrBits);

            if((addr & 0b11) != 0) {
                if(opcode == Opcode::LR_W) {
                    handleException(ExceptionCode::LOAD_MISALIGNED_EXC, addr);
                } else {
                    handleException(ExceptionCode::STR_AMO_MISALIGNED_EXC, addr);
                }
                break;
            }

            if(opcode == Opcode::LR_W) {
                if(!translateAddress(addr, MemoryAccessType::READ)) {
                    handleException(ExceptionCode::LOAD_PAGE_FAULT_EXC, addr);
                    break;
                }
            } else {
                if(!translateAddress(addr, MemoryAccessType::WRITE)) {
                    handleException(ExceptionCode::STR_AMO_PAGE_FAULT_EXC, addr);
                    break;
                }
            }

            uint32_t val = mem.readWord(addr);

            switch(opcode) {
                case Opcode::LR_W: {
                    gpr.setRegister(fmtR.rd, val);
                    reservationSetValid = true;
                    break;
                }
                case Opcode::SC_W: {
                    if(reservationSetValid) {
                        mem.writeWord(addr, gpr.getRegister(fmtR.rs2));
                        gpr.setRegister(fmtR.rd, 0);
                        reservationSetValid = false;
                    } else {
                        gpr.setRegister(fmtR.rd, 1);
                    }
                    break;
                }
                case Opcode::AMOSWAP_W: {
                    uint32_t temp = gpr.getRegister(fmtR.rs2);
                    gpr.setRegister(fmtR.rs2, val);
                    mem.writeWord(addr, temp);
                    gpr.setRegister(fmtR.rd, val);
                    break;
                }
                case Opcode::AMOADD_W: {
                    uint32_t temp = val + gpr.getRegister(fmtR.rs2);
                    mem.writeWord(addr, temp);
                    gpr.setRegister(fmtR.rd, val);
                    break;
                }
                case Opcode::AMOXOR_W: {
                    uint32_t temp = val ^ gpr.getRegister(fmtR.rs2);
                    mem.writeWord(addr, temp);
                    gpr.setRegister(fmtR.rd, val);
                    break;
                }
                case Opcode::AMOAND_W: {
                    uint32_t temp = val & gpr.getRegister(fmtR.rs2);
                    mem.writeWord(addr, temp);
                    gpr.setRegister(fmtR.rd, val);
                    break;
                }
                case Opcode::AMOOR_W: {
                    uint32_t temp = val | gpr.getRegister(fmtR.rs2);
                    mem.writeWord(addr, temp);
                    gpr.setRegister(fmtR.rd, val);
                    break;
                }
                case Opcode::AMOMIN_W: {
                    uint32_t temp = std::min(static_cast<int32_t>(val), static_cast<int32_t>(gpr.getRegister(fmtR.rs2)));
                    mem.writeWord(addr, temp);
                    gpr.setRegister(fmtR.rd, val);
                    break;
                }
                case Opcode::AMOMAX_W: {
                    uint32_t temp = std::max(static_cast<int32_t>(val), static_cast<int32_t>(gpr.getRegister(fmtR.rs2)));
                    mem.writeWord(addr, temp);
                    gpr.setRegister(fmtR.rd, val);
                    break;
                }
                case Opcode::AMOMINU_W: {
                    uint32_t temp = std::min(val, gpr.getRegister(fmtR.rs2));
                    mem.writeWord(addr, temp);
                    gpr.setRegister(fmtR.rd, val);
                    break;
                }
                case Opcode::AMOMAXU_W: {
                    uint32_t temp = std::max(val, gpr.getRegister(fmtR.rs2));
                    mem.writeWord(addr, temp);
                    gpr.setRegister(fmtR.rd, val);
                    break;
                }
            }
            break;
        }
        case Type::OP_IMM: {
                InstructionIType fmtI = instrBits;
                InstructionRType fmtR = instrBits;
                switch(Decoder::getOpcode(instrBits)) {
                    case Opcode::ADDI: {
                        gpr.setRegister(fmtI.rd, gpr.getRegister(fmtI.rs1) + fmtI.getImmediateValue());
                        break;
                    }
                    case Opcode::SLTI: {
                        int32_t rs1Signed = gpr.getRegister(fmtI.rs1);
                        int32_t immSigned = fmtI.getImmediateValue();
                        gpr.setRegister(fmtI.rd, (rs1Signed < immSigned) ? 1 : 0);
                        break;
                    }
                    case Opcode::SLTIU: {
                        uint32_t rs1Unsigned = gpr.getRegister(fmtI.rs1);
                        uint32_t immUnsigned = fmtI.getImmediateValue();
                        gpr.setRegister(fmtI.rd, (rs1Unsigned < immUnsigned) ? 1 : 0);
                        break;
                    }
                    case Opcode::XORI: {
                        gpr.setRegister(fmtI.rd, gpr.getRegister(fmtI.rs1) ^ fmtI.getImmediateValue());
                        break;
                    }
                    case Opcode::ORI: {
                        gpr.setRegister(fmtI.rd, gpr.getRegister(fmtI.rs1) | fmtI.getImmediateValue());
                        break;
                    }
                    case Opcode::ANDI: {
                        gpr.setRegister(fmtI.rd, gpr.getRegister(fmtI.rs1) & fmtI.getImmediateValue());
                        break;
                    }
                    case Opcode::SLLI: {
                        uint32_t shamt = fmtR.rs2;
                        gpr.setRegister(fmtI.rd, gpr.getRegister(fmtI.rs1) << shamt);
                        break;
                    }
                    case Opcode::SRLI: {
                        uint32_t shamt = fmtR.rs2;
                        gpr.setRegister(fmtI.rd, gpr.getRegister(fmtI.rs1) >> shamt);
                        break;
                    }
                    case Opcode::SRAI: {
                        uint32_t shamt = fmtR.rs2;
                        gpr.setRegister(fmtI.rd, static_cast<int32_t>(gpr.getRegister(fmtI.rs1)) >> shamt);
                        break;
                    }
                }
            }
            break;
        case Type::OP: {
            InstructionRType fmtR = instrBits;
            switch(Decoder::getOpcode(instrBits)) {
                case Opcode::ADD: {
                    gpr.setRegister(fmtR.rd, gpr.getRegister(fmtR.rs1) + gpr.getRegister(fmtR.rs2));
                    break;
                }
                case Opcode::SUB: {
                    gpr.setRegister(fmtR.rd, gpr.getRegister(fmtR.rs1) - gpr.getRegister(fmtR.rs2));
                    break;
                }
                case Opcode::SLL: {
                    uint32_t shamt = gpr.getRegister(fmtR.rs2) & 0x1F; // only use lower five bits for shift amount
                    gpr.setRegister(fmtR.rd, gpr.getRegister(fmtR.rs1) << shamt);
                    break;
                }
                case Opcode::SLT: {
                    int32_t rs1Signed = gpr.getRegister(fmtR.rs1);
                    int32_t rs2Signed = gpr.getRegister(fmtR.rs2);
                    gpr.setRegister(fmtR.rd, (rs1Signed < rs2Signed) ? 1 : 0);
                    break;
                }
                case Opcode::SLTU: {
                    uint32_t rs1 = gpr.getRegister(fmtR.rs1);
                    uint32_t rs2 = gpr.getRegister(fmtR.rs2);
                    gpr.setRegister(fmtR.rd, (rs1 < rs2) ? 1 : 0);
                    break;
                }
                case Opcode::XOR: {
                    gpr.setRegister(fmtR.rd, gpr.getRegister(fmtR.rs1) ^ gpr.getRegister(fmtR.rs2));
                    break;
                }
                case Opcode::SRL: {
                    uint32_t shamt = gpr.getRegister(fmtR.rs2) & 0x1F;
                    gpr.setRegister(fmtR.rd, gpr.getRegister(fmtR.rs1) >> shamt);
                    break;
                }
                case Opcode::SRA: {
                    uint32_t shamt = gpr.getRegister(fmtR.rs2) & 0x1F;
                    gpr.setRegister(fmtR.rd, static_cast<int32_t>(gpr.getRegister(fmtR.rs1)) >> shamt);
                    break;
                }
                case Opcode::OR: {
                    gpr.setRegister(fmtR.rd, gpr.getRegister(fmtR.rs1) | gpr.getRegister(fmtR.rs2));
                    break;
                }
                case Opcode::AND: {
                    gpr.setRegister(fmtR.rd, gpr.getRegister(fmtR.rs1) & gpr.getRegister(fmtR.rs2));
                    break;
                }
                case Opcode::MUL: {
                    uint64_t result = gpr.getRegister(fmtR.rs1) * gpr.getRegister(fmtR.rs2);
                    gpr.setRegister(fmtR.rd, result & 0xFFFFFFFF);
                    break;
                }
                case Opcode::MULH: {
                    uint64_t result = SIGN_EXTEND(static_cast<int64_t>(gpr.getRegister(fmtR.rs1)), 32) * SIGN_EXTEND(static_cast<int64_t>(gpr.getRegister(fmtR.rs2)), 32);
                    gpr.setRegister(fmtR.rd, (result >> 32) & 0xFFFFFFFF);
                    break;
                }
                case Opcode::MULHU: {
                    uint64_t result = static_cast<uint64_t>(gpr.getRegister(fmtR.rs1)) * static_cast<uint64_t>(gpr.getRegister(fmtR.rs2));
                    gpr.setRegister(fmtR.rd, (result >> 32) & 0xFFFFFFFF);
                    break;
                }
                case Opcode::MULHSU: {
                    uint64_t result = SIGN_EXTEND(static_cast<int64_t>(gpr.getRegister(fmtR.rs1)), 32) * static_cast<uint64_t>(gpr.getRegister(fmtR.rs2));
                    gpr.setRegister(fmtR.rd, (result >> 32) & 0xFFFFFFFF);
                    break;
                }
                case Opcode::DIV: {
                    uint32_t dividend = gpr.getRegister(fmtR.rs1);
                    uint32_t divisor = gpr.getRegister(fmtR.rs2);

                    // If dividend is most negative value and divisor is -1, result is dividend
                    if(dividend == 0x80000000 && divisor == 0xFFFFFFFF) {
                        gpr.setRegister(fmtR.rd, dividend);
                    } else if(divisor != 0) {
                        gpr.setRegister(fmtR.rd, static_cast<int32_t>(dividend) / static_cast<int32_t>(divisor));
                    } else {
                        gpr.setRegister(fmtR.rd, 0xFFFFFFFF); // result is -1 if division by zero
                    }
                    break;
                }
                case Opcode::DIVU: {
                    uint32_t divisor = gpr.getRegister(fmtR.rs2);

                    if(divisor != 0) {
                        gpr.setRegister(fmtR.rd, gpr.getRegister(fmtR.rs1) / divisor);
                    } else {
                        gpr.setRegister(fmtR.rd, 0xFFFFFFFF); // result is maximum unsigned value
                    }
                    break;
                }
                case Opcode::REM: {
                    uint32_t dividend = gpr.getRegister(fmtR.rs1);
                    uint32_t divisor = gpr.getRegister(fmtR.rs2);

                    // If divident is most negative value and divisor is -1, result is zero
                    if(dividend == 0x80000000 && divisor == 0xFFFFFFFF) {
                        gpr.setRegister(fmtR.rd, 0);
                    } else if(divisor != 0) {
                        gpr.setRegister(fmtR.rd, static_cast<int32_t>(dividend) % static_cast<int32_t>(divisor));
                    } else {
                        gpr.setRegister(fmtR.rd, dividend); // result is -1 if dividend
                    }
                    break;
                }
                case Opcode::REMU: {
                    uint32_t dividend = gpr.getRegister(fmtR.rs1);
                    uint32_t divisor = gpr.getRegister(fmtR.rs2);

                    if(divisor != 0) {
                        gpr.setRegister(fmtR.rd, dividend % divisor);
                    } else {
                        gpr.setRegister(fmtR.rd, dividend); // result is dividend
                    }
                    break;
                }
            }
        }
        break;
        case Type::SYSTEM: {
            InstructionIType fmtI = instrBits;

            bool skip = false;
            switch(Decoder::getOpcode(instrBits)) {
                case Opcode::SRET:
                    skip = true;
                    shouldIncrementPC = false;

                    supervisorMode = csr.sstatus.spp;
                    csr.sstatus.sie = csr.sstatus.spie;
                    csr.sstatus.spie = 1;
                    csr.sstatus.spp = 0; // set spp to user mode
                    pc = csr.sepc;
                    break;
                case Opcode::ECALL: {
                    skip = true;

                    uint32_t a0Val = gpr.getRegister(10);
                    uint32_t a1Val = gpr.getRegister(11);
                    uint32_t a7Val = gpr.getRegister(17);

                    if(supervisorMode) {
                        // Handle SBI call here
                        switch(a7Val) {
                            case 0: // SBI_SET_TIMER
                                timeCompare = (static_cast<uint64_t>(a1Val) << 32) | a0Val;
                                csr.sip.stip = 0; // clear timer interrupt bit

                                // Return SBI_SUCCESS in a0
                                break;
                            case 1: // SBI_CONSOLE_PUTCHAR
                                putCharCallback(static_cast<char>(a0Val));
                                break;
                            case 2: // SBI_CONSOLE_GETCHAR
                                // Register a0 contains the return value
                                gpr.setRegister(10, static_cast<uint32_t>(getCharCallback()));
                                break;
                            case 8: // SBI_SHUTDOWN
                                shutdownCallback();
                                // Return SBI_SUCCESS in a0
                                gpr.setRegister(10, 0);
                                break;
                            // case 93:
                            //     if(gpr.getRegister(3) == 1) {
                            //         std::cout << "TEST PASSED" << std::endl;
                            //     } else {
                            //         std::cout << "TEST FAILED" << std::endl;
                            //         std::cout << "gp = " << std::hex << gpr.getRegister(3) << std::endl;
                            //     }
                            //     shutdownCallback();
                            //     break;
                            default:
                                throw EmulatorException("Unknown SBI call " + std::to_string(a7Val));
                        }
                    } else {
                        handleException(ExceptionCode::U_ECALL_EXC, 0);
                    }
                    break;
                }
                case Opcode::SFENCE_VMA:
                case Opcode::SFENCE_INVAL_IR:
                case Opcode::SFENCE_W_INVAL:
                case Opcode::WFI:
                    skip = true;
                default:
                    break;
            }

            if(skip) {
                break;
            }

            uint32_t rs1Value = gpr.getRegister(fmtI.rs1);
            uint32_t csrField = fmtI.getCSRField();

            bool readcheck = !supervisorMode && csr[csrField].getAccessType() == CSRAccessType::SRW;
            bool permissionCheck1 = csr[csrField].getAccessType() == CSRAccessType::URO;
            bool permissionCheck2 = rs1Value != 0 && permissionCheck1;

            // scounteren determines if U-mode can read cycle, time, instret
            if(!supervisorMode) {
                if((csr.scounteren.cy == 0) && (csrField == csr.cycle.getAddress()
                    || csrField == csr.cycleh.getAddress())) {
                        handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instrBits);
                       break;   
                }
                if((csr.scounteren.tm == 0) && (csrField == csr.time.getAddress()
                    || csrField == csr.timeh.getAddress())) {
                        handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instrBits);
                       break;   
                }
                if((csr.scounteren.ir == 0) && (csrField == csr.instret.getAddress()
                    || csrField == csr.instreth.getAddress())) {
                        handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instrBits);
                       break;   
                }
            }

            if(readcheck) {
                handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instrBits);
                break; 
            }

            gpr.setRegister(fmtI.rd, csr[csrField]);

            switch(Decoder::getOpcode(instrBits)) {
                case Opcode::CSRRW:
                    if(permissionCheck1){
                       handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instrBits);
                       break; 
                    }
                    csr[csrField] = rs1Value;
                    break;
                case Opcode::CSRRS:
                    if(permissionCheck2){
                       handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instrBits);
                       break; 
                    }
                    csr[csrField] |= rs1Value;
                    break;
                case Opcode::CSRRC:
                    if(permissionCheck2){
                       handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instrBits);
                       break; 
                    }
                    csr[csrField] &= (~rs1Value);
                    break;
                case Opcode::CSRRWI:
                    if(permissionCheck1){
                       handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instrBits);
                       break; 
                    }
                    csr[csrField] = fmtI.rs1;
                    break;
                case Opcode::CSRRSI:
                    if(permissionCheck2){
                       handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instrBits);
                       break; 
                    }
                    csr[csrField] |= fmtI.rs1;
                    break;
                case Opcode::CSRRCI:
                    if(permissionCheck2){
                       handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instrBits);
                       break; 
                    }
                    csr[csrField] &= (~fmtI.rs1);
                    break;
            }
            break;
        }
        case Type::OP_UI: {
            InstructionUType fmtU = instrBits;
            switch(Decoder::getOpcode(instrBits)) {
                case Opcode::LUI:
                    gpr.setRegister(fmtU.rd, fmtU.getImmediateValue());
                    break;
                case Opcode::AUIPC:
                    gpr.setRegister(fmtU.rd, pc + fmtU.getImmediateValue());
                    break;
            }
            break;
        }
        case Type::OP_FENCE:
            break;
        default:
            throw EmulatorException("Unknown instruction " + std::to_string(instrBits));
    }

    if(shouldIncrementPC) {
        pc += 4;
    }

    incrementTimer();
}

void HartRV32::incrementTimer() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    uint64_t duration = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - lastTime).count();

    if(duration >= timebasePeriod) {
        uint64_t timerVal = (static_cast<uint64_t>(csr.timeh) << 32) | static_cast<uint64_t>(csr.time);
        timerVal += (duration / timebasePeriod);

        // csr.sip.stip = (timerVal >= timeCompare) ? 1 : 0;
        if(timerVal >= timeCompare) {
            csr.sip.stip = 1;
        }

        csr.time.setValue(timerVal & 0xFFFFFFFF);
        csr.timeh.setValue(timerVal >> 32);

        lastTime = currentTime;
    }

    if(csr.cycle == 0xFFFFFFFF) {
        csr.cycleh.setValue(csr.cycleh + 1);
    }
    if(csr.instret == 0xFFFFFFFF) {
        csr.instreth.setValue(csr.instreth + 1);
    }
    csr.cycle.setValue(csr.cycle + 1);
    csr.instret.setValue(csr.instret + 1);
}


void HartRV32::handleException(ExceptionCode code, uint32_t stval) {
        csr.sstatus.spp = supervisorMode;
        csr.sstatus.spie = csr.sstatus.sie;
        supervisorMode = true;

        csr.sstatus.sie = 0; // Clear sie

        // Write to sepc
        csr.sepc.setValue(pc);

        // Write exception code
        csr.scause.exceptionCode = static_cast<uint32_t>(code);

        // This is not an interrupt
        csr.scause.interrupt = 0;

        // Write to stval
        csr.stval.setValue(stval);

        // Set the PC
        pc = (csr.stvec.base << 2);
        shouldIncrementPC = false;
}

void HartRV32::handleInterrupt() {
    if(!supervisorMode || (supervisorMode && csr.sstatus.sie == 1)) {
        // Write exception code to scause
        if((csr.sip.ssip & csr.sie.ssie) != 0) {
            csr.scause.exceptionCode = 1; // supervisor software interrupt
        }
        else if((csr.sip.stip & csr.sie.stie) != 0) {
            csr.scause.exceptionCode = 5; // supervisor timer interrupt
        }
        else if((csr.sip.seip & csr.sie.seie) != 0) {
            csr.scause.exceptionCode = 9; // supervisor external interrupt
        } else {
            // If the interrupt(s) that are pending do not match the 
            // enabled interrupt(s), do not service the interrupt 
            return;
        }

        csr.sstatus.spp = supervisorMode;
        csr.sstatus.spie = csr.sstatus.sie;
        supervisorMode = true;

        csr.sstatus.sie = 0; // Clear sie

        // Write to sepc
        csr.sepc.setValue(pc);

        // Indicate this is an interrupt
        csr.scause.interrupt = 1;

        // Clear stval for interrupts
        csr.stval.setValue(0);

        // Finally, set PC
        shouldIncrementPC = false;
        pc = (csr.stvec.base << 2);
        if(csr.stvec.mode == 1) {
            pc += 4 * csr.scause.exceptionCode;
        }
    }
}

bool HartRV32::translateAddress(uint32_t &addr, MemoryAccessType accessType) {
    // Check if Sv32 paging is enabled
    if(csr.satp.mode == 0) {
        return true;
    }
    
    Sv32PTE pte {0};
    uint64_t base = csr.satp.ppn * PAGE_SIZE;
    Sv32VirtualAddr vAddr = addr;
    Sv32PhysAddr result {0};

    for(uint32_t i = 1; i >= 0; --i) {
        pte = mem.readWord(base + (i == 0 ? vAddr.vpn0 : vAddr.vpn1) * PTE_SIZE);

        if(pte.v == 0 || (pte.r == 0 && pte.w == 1)) {
            return false;
        }

        if(pte.r == 1 || pte.x == 1) {
            // U bit must be set for user mode software
            if(!supervisorMode && pte.u == 0) {
                return false;
            }
            // Check write and execute permissions
            if((pte.w == 0 && accessType == MemoryAccessType::WRITE) || (pte.x == 0 && accessType == MemoryAccessType::EXECUTE)) {
                return false;
            }
            // Check read permission using MXR bit rules
            if(accessType == MemoryAccessType::READ && ((csr.sstatus.mxr == 0 && pte.r == 0) || (pte.r == 0 && pte.x == 0))) {
                return false;
            }
            // Supervisor mode may not execute user pages
            if(accessType == MemoryAccessType::EXECUTE && supervisorMode && pte.u == 1) {
                return false;
            }
            // Check SUM bit to determine if supervisor mode can read/write user pages
            if((accessType != MemoryAccessType::EXECUTE) && supervisorMode && pte.u == 1 && csr.sstatus.sum == 0) {
                return false;
            }
            // Check for misaligned superpage
            if(i == 1 && pte.ppn0 != 0) {
                return false;
            }
            // Signal page fault to hardware for A and D bits
            if(pte.a == 0 || (accessType == MemoryAccessType::WRITE && pte.d == 0)) {
                return false;
            }
            
            result.pageOffset = vAddr.pageOffset;
            result.ppn0 = pte.ppn0;
            result.ppn1 = pte.ppn1;

            // If superpage use vpn0
            if(i == 1) {
                result.ppn0 = vAddr.vpn0;
            }

            break;
        } else {
            base = ((pte.ppn1 << 10) | pte.ppn0) * PAGE_SIZE;
        }
    }

    if(result.value == 0) {
        return false;
    }

    if((result.value & 0x300000000) != 0) {
        throw EmulatorException("Tried to access beyond 32 bits of physical memory");
    }
    // Disregard the upper two bits of the 34 bit Sv32 physical address
    addr = result.value & 0xFFFFFFFF;
    return true;
}

void HartRV32::reset() {
    gpr.reset();
}

void HartRV32::RegisterFile::reset() {
    for(size_t i = 0; i < NUM_GPR; ++i) {
        setRegister(i, 0);
    }
}

uint32_t HartRV32::RegisterFile::getRegister(uint32_t index) const {
    if(index == 0) {
        return 0;
    }
    return gpr_array[index];
}

void HartRV32::RegisterFile::setRegister(uint32_t index, uint32_t value) {
    if(index != 0) {
        gpr_array[index] = value;
    }
}
