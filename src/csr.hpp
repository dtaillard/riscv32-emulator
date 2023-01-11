#ifndef __CSR_HPP__
#define __CSR_HPP__

#include <cstdint>
#include <unordered_map>
#include "bit_field.hpp"
#include "emulator_exception.hpp"

enum class CSRAccessType {
    URW, URO, SRW
};

class CSRBase {
public:
    CSRBase() = delete;
    
    virtual CSRBase& operator=(uint32_t value) {
        this->value = value;
        return *this;
    }

    CSRBase& operator|=(uint32_t value) {
        this->value |= value;
        return *this;
    }

    CSRBase& operator&=(uint32_t value) {
        this->value &= value;
        return *this;
    }

    operator uint32_t() {
        return value;
    }

    uint32_t getAddress() const { return addr; }
    uint32_t getValue() const {return value; }
    void setValue(uint32_t value) { this->value = value; } // assignment operator workaround for now
    CSRAccessType getAccessType() { return accessType; }

protected:
    CSRBase(uint32_t value, uint32_t addr, CSRAccessType accessType)
        : addr(addr), value(value), accessType(accessType) {}
    
    const uint32_t addr;
    uint32_t value;
    const CSRAccessType accessType;
};

struct CSRFile {
    class Cycle: public CSRBase {
    public:
        Cycle() : CSRBase(0, 0xc00, CSRAccessType::URO) {}
    } cycle;
    
    class Time: public CSRBase {
    public:
        Time() : CSRBase(0, 0xc01, CSRAccessType::URO) {}
    } time;

    struct Instret: public CSRBase {
        Instret() : CSRBase(0, 0xc02, CSRAccessType::URO) {}
    } instret;

    struct Sstatus: public CSRBase {
        Sstatus() : CSRBase(0, 0x100, CSRAccessType::SRW) {}

        U32BitField<31, 31> sd   {value};
        U32BitField<19, 19> mxr  {value};
        U32BitField<18, 18> sum  {value};
        U32BitField<15, 16> xs   {value};
        U32BitField<13, 14> fs   {value};
        U32BitField<9, 10>  vs   {value};
        U32BitField<8, 8>   spp  {value};
        U32BitField<6, 6>   ube  {value};
        U32BitField<5, 5>   spie {value};
        U32BitField<1, 1>   sie  {value};
    } sstatus;

    struct Sie: public CSRBase {
        Sie() : CSRBase(0, 0x104, CSRAccessType::SRW) {}

        U32BitField<1, 1>   ssie {value};
        U32BitField<5, 5>   stie {value};
        U32BitField<9, 9>   seie {value};
    } sie;

    struct Stvec : public CSRBase {
        Stvec() : CSRBase(0, 0x105, CSRAccessType::SRW) {}

        U32BitField<0, 1>   mode {value};
        U32BitField<2, 31>  base {value};
    } stvec;

    // Linux probably never uses this...
    struct Scounteren : public CSRBase {
        Scounteren() : CSRBase(0, 0x106, CSRAccessType::SRW) {}

        U32BitField<0, 0>   cy {value};
        U32BitField<1, 1>   tm {value};
        U32BitField<2, 2>   ir {value};
    } scounteren;

    struct Sscratch: public CSRBase {
        Sscratch() : CSRBase(0, 0x140, CSRAccessType::SRW) {}
    } sscratch;

    struct Sepc: public CSRBase {
        Sepc() : CSRBase(0, 0x141, CSRAccessType::SRW) {}
    } sepc;

    struct Scause: public CSRBase {
        Scause() : CSRBase(0, 0x142, CSRAccessType::SRW) {}

        U32BitField<31, 31>   interrupt      {value};
        U32BitField<0, 30>   exceptionCode  {value};
    } scause;

    struct Stval: public CSRBase {
        Stval() : CSRBase(0, 0x143, CSRAccessType::SRW) {}

        Stval& operator=(Stval& other) = delete;
    } stval;

    struct Sip: public CSRBase {
        Sip() : CSRBase(0, 0x144, CSRAccessType::SRW) {}

        U32BitField<1, 1>   ssip {value};
        U32BitField<5, 5>   stip {value};
        U32BitField<9, 9>   seip {value};
    } sip;

    struct Satp: public CSRBase {
        Satp() : CSRBase(0, 0x180, CSRAccessType::SRW) {}

        U32BitField<0, 21>    ppn  {value};
        U32BitField<22, 30>   asid {value};
        U32BitField<31, 31>   mode {value};
    } satp;

    class CycleH: public CSRBase {
    public:
        CycleH() : CSRBase(0, 0xc80, CSRAccessType::URO) {}
    } cycleh;

    class TimeH: public CSRBase {
    public:
        TimeH() : CSRBase(0, 0xc81, CSRAccessType::URO) {}
    } timeh;

    struct InstretH: public CSRBase {
        InstretH() : CSRBase(0, 0xc82, CSRAccessType::URO) {}
    } instreth;

    CSRBase& operator[](uint32_t addr) {
        if(csrMap.find(addr) == csrMap.end()) {
            throw EmulatorException("Unknown CSR " + std::to_string(addr));
        }
        return *csrMap[addr];
    }

    protected:
        std::unordered_map<uint32_t, CSRBase*> csrMap = {{
            {cycle.getAddress(),        &cycle},
            {time.getAddress(),         &time},
            {instret.getAddress(),      &instret},
            {sstatus.getAddress(),      &sstatus},
            {sie.getAddress(),          &sie},
            {stvec.getAddress(),        &stvec},
            {scounteren.getAddress(),   &scounteren},
            {sscratch.getAddress(),     &sscratch},
            {sepc.getAddress(),         &sepc},
            {scause.getAddress(),       &scause},
            {stval.getAddress(),        &stval},
            {sip.getAddress(),          &sip},
            {satp.getAddress(),         &satp},
            {cycleh.getAddress(),       &cycleh},
            {timeh.getAddress(),        &timeh},
            {instreth.getAddress(),     &instreth},
        }};
};

#endif /* __CSR_HPP__ */
