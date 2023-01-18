#ifndef __PTE_HPP__
#define __PTE_HPP__

#include "bit_field.hpp"

constexpr uint32_t PAGE_SIZE = 4096;
constexpr uint32_t PTE_SIZE = 4;

union Sv32VirtualAddr {
    uint32_t bits;

    U32BitField<0, 11>  pageOffset;
    U32BitField<12, 21> vpn0;
    U32BitField<22, 31> vpn1;
};

union Sv32PhysAddr {
    uint64_t bits;

    U64BitField<0, 11>  pageOffset;
    U64BitField<12, 21> ppn0;
    U64BitField<22, 33> ppn1;
};

union Sv32PTE {
    uint64_t bits;

    U64BitField<0, 0>   v;
    U64BitField<1, 1>   r;
    U64BitField<2, 2>   w;
    U64BitField<3, 3>   x;
    U64BitField<4, 4>   u;
    U64BitField<5, 5>   g;
    U64BitField<6, 6>   a;
    U64BitField<7, 7>   d;
    U64BitField<8, 9>   rsw;
    U64BitField<10, 19> ppn0;
    U64BitField<20, 31> ppn1;
};

#endif /* __PTE_HPP__ */
