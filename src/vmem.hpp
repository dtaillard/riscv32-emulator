#ifndef __PTE_HPP__
#define __PTE_HPP__

#include "bit_field.hpp"

constexpr uint32_t PAGE_SIZE = 4096;
constexpr uint32_t PTE_SIZE = 4;

struct Sv32VirtualAddr {
    Sv32VirtualAddr(uint32_t value): value(value) {}

    uint32_t value;

    U32BitField<0, 11>  pageOffset  {value};
    U32BitField<12, 21> vpn0        {value};
    U32BitField<22, 31> vpn1        {value};
};

struct Sv32PhysAddr {
    Sv32PhysAddr(uint64_t value): value(value) {}

    uint64_t value;

    U64BitField<0, 11>  pageOffset  {value};
    U64BitField<12, 21> ppn0        {value};
    U64BitField<22, 33> ppn1        {value};
};

struct Sv32PTE {
    Sv32PTE(uint64_t value): value(value) {}

    uint64_t value;

    U64BitField<0, 0>  v            {value};
    U64BitField<1, 1>  r            {value};
    U64BitField<2, 2>  w            {value};
    U64BitField<3, 3>  x            {value};
    U64BitField<4, 4>  u            {value};
    U64BitField<5, 5>  g            {value};
    U64BitField<6, 6>  a            {value};
    U64BitField<7, 7>  d            {value};
    U64BitField<8, 9>  rsw          {value};
    U64BitField<10, 19> ppn0        {value};
    U64BitField<20, 31> ppn1        {value};
};

#endif /* __PTE_HPP__ */
