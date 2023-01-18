#ifndef __CSR_HPP__
#define __CSR_HPP__

#include <cstdint>
#include <unordered_map>
#include "bit_field.hpp"
#include "emulator_exception.hpp"

enum class CSRAccessType: uint32_t {
    URW, URO, SRW
};

enum class CSRAddress: uint32_t {
    CYCLE = 0xC00,
    TIME = 0xC01,
    INSTRET = 0xC02,
    SSTATUS = 0x100,
    SIE = 0x104,
    STVEC = 0x105,
    SCOUNTEREN = 0x106,
    SSCRATCH = 0x140,
    SEPC = 0x141,
    SCAUSE = 0x142,
    STVAL = 0x143,
    SIP = 0x144,
    SATP = 0x180,
    CYCLEH = 0xc80,
    TIMEH = 0xc81,
    INSTRETH = 0xc82,
};

struct CSRs {
    uint32_t cycle;
    uint32_t cycleh;

    uint32_t time;
    uint32_t timeh;

    uint32_t instret;
    uint32_t instreth;
    
    uint32_t sscratch;
    uint32_t sepc;
    uint32_t stval;

    union {
        uint32_t bits;

        U32BitField<31, 31>  sd;
        U32BitField<19, 19>  mxr;
        U32BitField<18, 18>  sum;
        U32BitField<15, 16>  xs;
        U32BitField<13, 14>  fs;
        U32BitField<9, 10>   vs;
        U32BitField<8, 8>    spp;
        U32BitField<6, 6>    ube;
        U32BitField<5, 5>    spie;
        U32BitField<1, 1>    sie;
    } sstatus;

    union {
        uint32_t bits;

        U32BitField<1, 1>    ssie;
        U32BitField<5, 5>    stie;
        U32BitField<9, 9>    seie;
    } sie;

    union {
        uint32_t bits;

        U32BitField<0, 1>    mode;
        U32BitField<2, 31>   base;
    } stvec;

    union {
        uint32_t bits;

        U32BitField<0, 0>    cy;
        U32BitField<1, 1>    tm;
        U32BitField<2, 2>    ir;
    } scounteren;

    union {
        uint32_t bits;

        U32BitField<31, 31>  interrupt;
        U32BitField<0, 30>   exceptionCode;
    } scause;

    union {
        uint32_t bits;

        U32BitField<1, 1>    ssip;
        U32BitField<5, 5>    stip;
        U32BitField<9, 9>    seip;
    } sip;

    union {
        uint32_t bits;

        U32BitField<0, 21>   ppn;
        U32BitField<22, 30>  asid;
        U32BitField<31, 31>  mode;
    } satp;

    uint32_t& operator[](uint32_t addr) {
        switch(CSRAddress(addr)) {
            case CSRAddress::CYCLE:
                return cycle;
            case CSRAddress::TIME:
                return time;
            case CSRAddress::INSTRET:
                return instret;
            case CSRAddress::SSTATUS:
                return sstatus.bits;
            case CSRAddress::SIE:
                return sie.bits;
            case CSRAddress::STVEC:
                return stvec.bits;
            case CSRAddress::SCOUNTEREN:
                return scounteren.bits;
            case CSRAddress::SSCRATCH:
                return sscratch;
            case CSRAddress::SEPC:
                return sepc;
            case CSRAddress::SCAUSE:
                return scause.bits;
            case CSRAddress::STVAL:
                return stval;
            case CSRAddress::SIP:
                return sip.bits;
            case CSRAddress::SATP:
                return satp.bits;
            case CSRAddress::CYCLEH:
                return cycleh;
            case CSRAddress::TIMEH:
                return timeh;
            case CSRAddress::INSTRETH:
                return instreth;
            default:
                throw EmulatorException("Unknown CSR " + std::to_string(addr));
        }
    }

    static constexpr CSRAccessType getAccessType(uint32_t addr) {
        switch(CSRAddress(addr)) {
            case CSRAddress::CYCLE:
            case CSRAddress::TIME:
            case CSRAddress::INSTRET:
            case CSRAddress::CYCLEH:
            case CSRAddress::TIMEH:
            case CSRAddress::INSTRETH:
                return CSRAccessType::URO;
            case CSRAddress::SSTATUS:
            case CSRAddress::SIE:
            case CSRAddress::STVEC:
            case CSRAddress::SCOUNTEREN:
            case CSRAddress::SSCRATCH:
            case CSRAddress::SEPC:
            case CSRAddress::SCAUSE:
            case CSRAddress::STVAL:
            case CSRAddress::SIP:
            case CSRAddress::SATP:
                return CSRAccessType::SRW;
            default:
                throw EmulatorException("Unknown CSR " + std::to_string(addr));
        }
    }
};

#endif /* __CSR_HPP__ */
