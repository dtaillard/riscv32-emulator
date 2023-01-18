#include "hart.hpp"
#include "emulator_exception.hpp"
#include "instruction.hpp"
#include "decoder.hpp"
#include "vmem.hpp"
#include "instruction.hpp"
#include <iostream>

using namespace RV32;

Hart::Hart(uint32_t pc, MemoryMapManager &mem, const HartConfig& hartConfig):
    pc(pc), mem(mem), hartConfig(hartConfig), lastTime(std::chrono::system_clock::now()), timebasePeriod(SECONDS_TO_NANSECONDS * (1.0 / hartConfig.timebaseFreq)) {
    this->reset();
}

uint32_t Hart::getRegister(uint32_t index) const {
    return gpr.r[index];
}

void Hart::setRegister(uint32_t index, uint32_t value) {
    gpr.r[index] = value;
    gpr.zero = 0;
}

void Hart::stepInstruction() {
    if((pc & 0b11) != 0) {
        handleException(ExceptionCode::INSTR_MISALIGNED_EXC, pc);
    }

    handleInterrupts();

    uint32_t pcPhysicalAddr = pc;
    if(!translateAddress(pcPhysicalAddr, MemoryAccessType::EXECUTE)) {
        handleException(ExceptionCode::INSTR_PAGE_FAULT_EXC, pc);
        pcPhysicalAddr = pc;
        if(!translateAddress(pcPhysicalAddr, MemoryAccessType::EXECUTE)) {
            throw EmulatorException("Page fault while translating exception handler address");
        }
    }

    Instruction instr { mem.readWord(pcPhysicalAddr) };
    shouldIncrementPC = true;

    switch(decodeInstructionType(instr)) {
        case InstructionType::LOAD: {
                uint32_t effectiveAddr = getRegister(instr.i.rs1) + instr.i.immediateValue();

                if(!translateAddress(effectiveAddr, MemoryAccessType::READ)) {
                    handleException(ExceptionCode::LOAD_PAGE_FAULT_EXC, effectiveAddr);
                    break;
                }

                switch(decodeOpcode(instr)) {
                    case Opcode::LB:
                        setRegister(instr.i.rd, SIGN_EXTEND(mem.readByte(effectiveAddr), 8));
                        break;
                    case Opcode::LH:
                        if((effectiveAddr & 0b1) != 0) {
                            handleException(ExceptionCode::LOAD_MISALIGNED_EXC, effectiveAddr);
                            break;
                        }
                        setRegister(instr.i.rd, SIGN_EXTEND(mem.readHalfword(effectiveAddr), 16));
                        break;
                    case Opcode::LW:
                        if((effectiveAddr & 0b11) != 0) {
                            handleException(ExceptionCode::LOAD_MISALIGNED_EXC, effectiveAddr);
                            break;
                        }
                        setRegister(instr.i.rd, mem.readWord(effectiveAddr));
                        break;
                    case Opcode::LBU:
                        setRegister(instr.i.rd, mem.readByte(effectiveAddr));
                        break;
                    case Opcode::LHU:
                        if((effectiveAddr & 0b1) != 0) {
                            handleException(ExceptionCode::LOAD_MISALIGNED_EXC, effectiveAddr);
                            break;
                        }
                        setRegister(instr.i.rd, mem.readHalfword(effectiveAddr));
                        break;
                }
            }
            break;
        case InstructionType::STORE: {
                uint32_t effectiveAddr = getRegister(instr.s.rs1) + instr.s.immediateValue();

                if(!translateAddress(effectiveAddr, MemoryAccessType::WRITE)) {
                    handleException(ExceptionCode::STR_AMO_PAGE_FAULT_EXC, effectiveAddr);
                    break;
                }

                switch(decodeOpcode(instr)) {
                    case Opcode::SB:
                        mem.writeByte(effectiveAddr, getRegister(instr.s.rs2) & 0xFF);
                        break;
                    case Opcode::SH:
                        if((effectiveAddr & 0b1) != 0) {
                            handleException(ExceptionCode::STR_AMO_MISALIGNED_EXC, effectiveAddr);
                            break;
                        }
                        mem.writeHalfword(effectiveAddr, getRegister(instr.s.rs2) & 0xFFFF);
                        break;
                    case Opcode::SW:
                        if((effectiveAddr & 0b11) != 0) {
                            handleException(ExceptionCode::STR_AMO_MISALIGNED_EXC, effectiveAddr);
                            break;
                        }
                        mem.writeWord(effectiveAddr, getRegister(instr.s.rs2));
                        break;
                }
            }
            break;
        case InstructionType::BRANCH: {
            uint32_t effectiveAddr = pc + instr.b.immediateValue();

            switch(decodeOpcode(instr)) {
                case Opcode::BEQ:
                    if(getRegister(instr.b.rs1) == getRegister(instr.b.rs2)) {
                        shouldIncrementPC = false;
                        pc = effectiveAddr;
                    }
                    break;
                case Opcode::BNE:
                    if(getRegister(instr.b.rs1) != getRegister(instr.b.rs2)) {
                        shouldIncrementPC = false;
                        pc = effectiveAddr;
                    }
                    break;
                case Opcode::BLT:
                    if(static_cast<int32_t>(getRegister(instr.b.rs1)) < static_cast<int32_t>(getRegister(instr.b.rs2))) {
                        shouldIncrementPC = false;
                        pc = effectiveAddr;
                    }
                    break;
                case Opcode::BGE:
                    if(static_cast<int32_t>(getRegister(instr.b.rs1)) >= static_cast<int32_t>(getRegister(instr.b.rs2))) {
                        shouldIncrementPC = false;
                        pc = effectiveAddr;
                    }
                    break;
                case Opcode::BLTU:
                    if(getRegister(instr.b.rs1) < getRegister(instr.b.rs2)) {
                        shouldIncrementPC = false;
                        pc = effectiveAddr;
                    }
                    break;
                case Opcode::BGEU:
                    if(getRegister(instr.b.rs1) >= getRegister(instr.b.rs2)) {
                        shouldIncrementPC = false;
                        pc = effectiveAddr;
                    }
                    break;
            }
            break;
        }
        case InstructionType::JUMP:
            shouldIncrementPC = false;
            switch(decodeOpcode(instr)) {
                    case Opcode::JAL: {
                        setRegister(instr.j.rd, pc + 4);
                        uint32_t effectiveAddr = pc + instr.j.immediateValue();
                        pc = effectiveAddr;
                        break;
                    }
                    case Opcode::JALR: {
                        uint32_t effectiveAddr = getRegister(instr.i.rs1) + instr.i.immediateValue();
                        setRegister(instr.i.rd, pc + 4);
                        pc = effectiveAddr & 0xFFFFFFFE; // clear least significant bit
                        break;
                    }
            }
            break;
        case InstructionType::AMO: {
            uint32_t addr = getRegister(instr.r.rs1);

            Opcode opcode = decodeOpcode(instr);

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
                    setRegister(instr.r.rd, val);
                    reservationSetValid = true;
                    break;
                }
                case Opcode::SC_W: {
                    if(reservationSetValid) {
                        mem.writeWord(addr, getRegister(instr.r.rs2));
                        setRegister(instr.r.rd, 0);
                        reservationSetValid = false;
                    } else {
                        setRegister(instr.r.rd, 1);
                    }
                    break;
                }
                case Opcode::AMOSWAP_W: {
                    uint32_t temp = getRegister(instr.r.rs2);
                    setRegister(instr.r.rs2, val);
                    mem.writeWord(addr, temp);
                    setRegister(instr.r.rd, val);
                    break;
                }
                case Opcode::AMOADD_W: {
                    uint32_t temp = val + getRegister(instr.r.rs2);
                    mem.writeWord(addr, temp);
                    setRegister(instr.r.rd, val);
                    break;
                }
                case Opcode::AMOXOR_W: {
                    uint32_t temp = val ^ getRegister(instr.r.rs2);
                    mem.writeWord(addr, temp);
                    setRegister(instr.r.rd, val);
                    break;
                }
                case Opcode::AMOAND_W: {
                    uint32_t temp = val & getRegister(instr.r.rs2);
                    mem.writeWord(addr, temp);
                    setRegister(instr.r.rd, val);
                    break;
                }
                case Opcode::AMOOR_W: {
                    uint32_t temp = val | getRegister(instr.r.rs2);
                    mem.writeWord(addr, temp);
                    setRegister(instr.r.rd, val);
                    break;
                }
                case Opcode::AMOMIN_W: {
                    uint32_t temp = std::min(static_cast<int32_t>(val), static_cast<int32_t>(getRegister(instr.r.rs2)));
                    mem.writeWord(addr, temp);
                    setRegister(instr.r.rd, val);
                    break;
                }
                case Opcode::AMOMAX_W: {
                    uint32_t temp = std::max(static_cast<int32_t>(val), static_cast<int32_t>(getRegister(instr.r.rs2)));
                    mem.writeWord(addr, temp);
                    setRegister(instr.r.rd, val);
                    break;
                }
                case Opcode::AMOMINU_W: {
                    uint32_t temp = std::min(val, getRegister(instr.r.rs2));
                    mem.writeWord(addr, temp);
                    setRegister(instr.r.rd, val);
                    break;
                }
                case Opcode::AMOMAXU_W: {
                    uint32_t temp = std::max(val, getRegister(instr.r.rs2));
                    mem.writeWord(addr, temp);
                    setRegister(instr.r.rd, val);
                    break;
                }
            }
            break;
        }
        case InstructionType::OP_IMM: {
                switch(decodeOpcode(instr)) {
                    case Opcode::ADDI: {
                        setRegister(instr.i.rd, getRegister(instr.i.rs1) + instr.i.immediateValue());
                        break;
                    }
                    case Opcode::SLTI: {
                        int32_t rs1Signed = getRegister(instr.i.rs1);
                        int32_t immSigned = instr.i.immediateValue();
                        setRegister(instr.i.rd, (rs1Signed < immSigned) ? 1 : 0);
                        break;
                    }
                    case Opcode::SLTIU: {
                        uint32_t rs1Unsigned = getRegister(instr.i.rs1);
                        uint32_t immUnsigned = instr.i.immediateValue();
                        setRegister(instr.i.rd, (rs1Unsigned < immUnsigned) ? 1 : 0);
                        break;
                    }
                    case Opcode::XORI: {
                        setRegister(instr.i.rd, getRegister(instr.i.rs1) ^ instr.i.immediateValue());
                        break;
                    }
                    case Opcode::ORI: {
                        setRegister(instr.i.rd, getRegister(instr.i.rs1) | instr.i.immediateValue());
                        break;
                    }
                    case Opcode::ANDI: {
                        setRegister(instr.i.rd, getRegister(instr.i.rs1) & instr.i.immediateValue());
                        break;
                    }
                    case Opcode::SLLI: {
                        uint32_t shamt = instr.r.rs2;
                        setRegister(instr.i.rd, getRegister(instr.i.rs1) << shamt);
                        break;
                    }
                    case Opcode::SRLI: {
                        uint32_t shamt = instr.r.rs2;
                        setRegister(instr.i.rd, getRegister(instr.i.rs1) >> shamt);
                        break;
                    }
                    case Opcode::SRAI: {
                        uint32_t shamt = instr.r.rs2;
                        setRegister(instr.i.rd, static_cast<int32_t>(getRegister(instr.i.rs1)) >> shamt);
                        break;
                    }
                }
            }
            break;
        case InstructionType::OP: {
            switch(decodeOpcode(instr)) {
                case Opcode::ADD: {
                    setRegister(instr.r.rd, getRegister(instr.r.rs1) + getRegister(instr.r.rs2));
                    break;
                }
                case Opcode::SUB: {
                    setRegister(instr.r.rd, getRegister(instr.r.rs1) - getRegister(instr.r.rs2));
                    break;
                }
                case Opcode::SLL: {
                    uint32_t shamt = getRegister(instr.r.rs2) & 0x1F; // only use lower five bits for shift amount
                    setRegister(instr.r.rd, getRegister(instr.r.rs1) << shamt);
                    break;
                }
                case Opcode::SLT: {
                    int32_t rs1Signed = getRegister(instr.r.rs1);
                    int32_t rs2Signed = getRegister(instr.r.rs2);
                    setRegister(instr.r.rd, (rs1Signed < rs2Signed) ? 1 : 0);
                    break;
                }
                case Opcode::SLTU: {
                    uint32_t rs1 = getRegister(instr.r.rs1);
                    uint32_t rs2 = getRegister(instr.r.rs2);
                    setRegister(instr.r.rd, (rs1 < rs2) ? 1 : 0);
                    break;
                }
                case Opcode::XOR: {
                    setRegister(instr.r.rd, getRegister(instr.r.rs1) ^ getRegister(instr.r.rs2));
                    break;
                }
                case Opcode::SRL: {
                    uint32_t shamt = getRegister(instr.r.rs2) & 0x1F;
                    setRegister(instr.r.rd, getRegister(instr.r.rs1) >> shamt);
                    break;
                }
                case Opcode::SRA: {
                    uint32_t shamt = getRegister(instr.r.rs2) & 0x1F;
                    setRegister(instr.r.rd, static_cast<int32_t>(getRegister(instr.r.rs1)) >> shamt);
                    break;
                }
                case Opcode::OR: {
                    setRegister(instr.r.rd, getRegister(instr.r.rs1) | getRegister(instr.r.rs2));
                    break;
                }
                case Opcode::AND: {
                    setRegister(instr.r.rd, getRegister(instr.r.rs1) & getRegister(instr.r.rs2));
                    break;
                }
                case Opcode::MUL: {
                    uint64_t result = getRegister(instr.r.rs1) * getRegister(instr.r.rs2);
                    setRegister(instr.r.rd, result & 0xFFFFFFFF);
                    break;
                }
                case Opcode::MULH: {
                    uint64_t result = SIGN_EXTEND(static_cast<int64_t>(getRegister(instr.r.rs1)), 32) * SIGN_EXTEND(static_cast<int64_t>(getRegister(instr.r.rs2)), 32);
                    setRegister(instr.r.rd, (result >> 32) & 0xFFFFFFFF);
                    break;
                }
                case Opcode::MULHU: {
                    uint64_t result = static_cast<uint64_t>(getRegister(instr.r.rs1)) * static_cast<uint64_t>(getRegister(instr.r.rs2));
                    setRegister(instr.r.rd, (result >> 32) & 0xFFFFFFFF);
                    break;
                }
                case Opcode::MULHSU: {
                    uint64_t result = SIGN_EXTEND(static_cast<int64_t>(getRegister(instr.r.rs1)), 32) * static_cast<uint64_t>(getRegister(instr.r.rs2));
                    setRegister(instr.r.rd, (result >> 32) & 0xFFFFFFFF);
                    break;
                }
                case Opcode::DIV: {
                    uint32_t dividend = getRegister(instr.r.rs1);
                    uint32_t divisor = getRegister(instr.r.rs2);

                    // If dividend is most negative value and divisor is -1, result is dividend
                    if(dividend == 0x80000000 && divisor == 0xFFFFFFFF) {
                        setRegister(instr.r.rd, dividend);
                    } else if(divisor != 0) {
                        setRegister(instr.r.rd, static_cast<int32_t>(dividend) / static_cast<int32_t>(divisor));
                    } else {
                        setRegister(instr.r.rd, 0xFFFFFFFF); // result is -1 if division by zero
                    }
                    break;
                }
                case Opcode::DIVU: {
                    uint32_t divisor = getRegister(instr.r.rs2);

                    if(divisor != 0) {
                        setRegister(instr.r.rd, getRegister(instr.r.rs1) / divisor);
                    } else {
                        setRegister(instr.r.rd, 0xFFFFFFFF); // result is maximum unsigned value
                    }
                    break;
                }
                case Opcode::REM: {
                    uint32_t dividend = getRegister(instr.r.rs1);
                    uint32_t divisor = getRegister(instr.r.rs2);

                    // If divident is most negative value and divisor is -1, result is zero
                    if(dividend == 0x80000000 && divisor == 0xFFFFFFFF) {
                        setRegister(instr.r.rd, 0);
                    } else if(divisor != 0) {
                        setRegister(instr.r.rd, static_cast<int32_t>(dividend) % static_cast<int32_t>(divisor));
                    } else {
                        setRegister(instr.r.rd, dividend); // result is -1 if dividend
                    }
                    break;
                }
                case Opcode::REMU: {
                    uint32_t dividend = getRegister(instr.r.rs1);
                    uint32_t divisor = getRegister(instr.r.rs2);

                    if(divisor != 0) {
                        setRegister(instr.r.rd, dividend % divisor);
                    } else {
                        setRegister(instr.r.rd, dividend); // result is dividend
                    }
                    break;
                }
            }
        }
        break;
        case InstructionType::SYSTEM: {
            bool skip = false;
            switch(decodeOpcode(instr)) {
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

                    if(supervisorMode) {
                        // Handle SBI call here
                        switch(gpr.a7) {
                            case 0: // SBI_SET_TIMER
                                timeCompare = (static_cast<uint64_t>(gpr.a1) << 32) | gpr.a0;
                                csr.sip.stip = 0;
                                break;
                            case 1: // SBI_CONSOLE_PUTCHAR
                                hartConfig.putCharCallback(static_cast<char>(gpr.a0));
                                break;
                            case 2: // SBI_CONSOLE_GETCHAR
                                gpr.a0 = static_cast<uint32_t>(hartConfig.getCharCallback());
                                break;
                            case 8: // SBI_SHUTDOWN
                                hartConfig.shutdownCallback();
                                gpr.a0 = 0;
                                break;
                            default:
                                throw EmulatorException("Unknown SBI call " + std::to_string(gpr.a7));
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

            uint32_t rs1Value = getRegister(instr.i.rs1);
            uint32_t csrField = instr.i.imm_11_0;

            bool readcheck = !supervisorMode && csr.getAccessType(csrField) == CSRAccessType::SRW;
            bool permissionCheck1 = csr.getAccessType(csrField) == CSRAccessType::URO;
            bool permissionCheck2 = rs1Value != 0 && permissionCheck1;

            // SCOUNTEREN determines if U-mode can read cycle, time, instret
            if(!supervisorMode) {
                if((csr.scounteren.cy == 0) && (CSRAddress(csrField) == CSRAddress::CYCLE || CSRAddress(csrField) == CSRAddress::CYCLEH)) {
                    handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instr.bits);
                    break;   
                }
                if((csr.scounteren.tm == 0) && (CSRAddress(csrField) == CSRAddress::TIME || CSRAddress(csrField) == CSRAddress::TIMEH)) {
                    handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instr.bits);
                    break;   
                }
                if((csr.scounteren.ir == 0) && (CSRAddress(csrField) == CSRAddress::INSTRET || CSRAddress(csrField) == CSRAddress::INSTRETH)) {
                    handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instr.bits);
                    break;   
                }
            }

            if(readcheck) {
                handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instr.bits);
                break; 
            }

            setRegister(instr.i.rd, csr[csrField]);

            switch(decodeOpcode(instr)) {
                case Opcode::CSRRW:
                    if(permissionCheck1){
                       handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instr.bits);
                       break; 
                    }
                    csr[csrField] = rs1Value;
                    break;
                case Opcode::CSRRS:
                    if(permissionCheck2){
                       handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instr.bits);
                       break; 
                    }
                    csr[csrField] |= rs1Value;
                    break;
                case Opcode::CSRRC:
                    if(permissionCheck2){
                       handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instr.bits);
                       break; 
                    }
                    csr[csrField] &= (~rs1Value);
                    break;
                case Opcode::CSRRWI:
                    if(permissionCheck1){
                       handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instr.bits);
                       break; 
                    }
                    csr[csrField] = instr.i.rs1;
                    break;
                case Opcode::CSRRSI:
                    if(permissionCheck2){
                       handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instr.bits);
                       break; 
                    }
                    csr[csrField] |= instr.i.rs1;
                    break;
                case Opcode::CSRRCI:
                    if(permissionCheck2){
                       handleException(ExceptionCode::INSTR_ILLEGAL_EXC, instr.bits);
                       break; 
                    }
                    csr[csrField] &= (~instr.i.rs1);
                    break;
            }
            break;
        }
        case InstructionType::OP_UI: {
            switch(decodeOpcode(instr)) {
                case Opcode::LUI:
                    setRegister(instr.u.rd, instr.u.immediateValue());
                    break;
                case Opcode::AUIPC:
                    setRegister(instr.u.rd, pc + instr.u.immediateValue());
                    break;
            }
            break;
        }
        case InstructionType::OP_FENCE:
            break;
        default:
            throw EmulatorException("Unknown instruction " + std::to_string(instr.bits));
    }

    if(shouldIncrementPC) {
        pc += 4;
    }

    incrementCounters();
}

void Hart::incrementCounters() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    uint64_t duration = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - lastTime).count();

    if(duration >= timebasePeriod) {
        uint64_t timerVal = (static_cast<uint64_t>(csr.timeh) << 32) | static_cast<uint64_t>(csr.time);
        timerVal += (duration / timebasePeriod);

        // csr.sip.stip = (timerVal >= timeCompare) ? 1 : 0;
        if(timerVal >= timeCompare) {
            csr.sip.stip = 1;
        }

        csr.time = timerVal & 0xFFFFFFFF;
        csr.timeh = timerVal >> 32;

        lastTime = currentTime;
    }

    if(csr.cycle == 0xFFFFFFFF) {
        csr.cycleh++;
    }
    if(csr.instret == 0xFFFFFFFF) {
        csr.instreth++;
    }
    csr.cycle++;
    csr.instret++;
}

void Hart::handleException(ExceptionCode code, uint32_t stval) {
        csr.sstatus.spp = supervisorMode;
        csr.sstatus.spie = csr.sstatus.sie;
        supervisorMode = true;

        csr.sstatus.sie = 0; // Clear sie

        // Write to sepc
        csr.sepc = pc;

        // Write exception code
        csr.scause.exceptionCode = static_cast<uint32_t>(code);

        // This is not an interrupt
        csr.scause.interrupt = 0;

        // Write to stval
        csr.stval = stval;

        // Set the PC
        pc = (csr.stvec.base << 2);
        shouldIncrementPC = false;
}

void Hart::handleInterrupts() {
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
        csr.sepc = pc;

        // Indicate this is an interrupt
        csr.scause.interrupt = 1;

        // Clear stval for interrupts
        csr.stval = 0;

        // Finally, set PC
        shouldIncrementPC = false;
        pc = (csr.stvec.base << 2);
        if(csr.stvec.mode == 1) {
            pc += 4 * csr.scause.exceptionCode;
        }
    }
}

bool Hart::translateAddress(uint32_t &addr, MemoryAccessType accessType) {
    // Check if Sv32 paging is enabled
    if(csr.satp.mode == 0) {
        return true;
    }
    
    Sv32PTE pte {};
    Sv32VirtualAddr vAddr = {addr};
    Sv32PhysAddr result {};
    uint64_t base = csr.satp.ppn * PAGE_SIZE;

    for(uint32_t i = 1; i >= 0; --i) {
        pte.bits = mem.readWord(base + (i == 0 ? vAddr.vpn0 : vAddr.vpn1) * PTE_SIZE);

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

    if(result.bits == 0) {
        return false;
    }

    if((result.bits & 0x300000000) != 0) {
        throw EmulatorException("Tried to access beyond 32 bits of physical memory");
    }

    // Disregard the upper two bits of the 34 bit Sv32 physical address
    addr = result.bits & 0xFFFFFFFF;
    return true;
}
