#ifndef __BIT_FIELD_HPP__
#define __BIT_FIELD_HPP__

#include <cstdint>
#include <cstddef>

template <typename T, size_t BEGIN, size_t END>
struct BitField {
    T bits;

    constexpr operator T() const {
        return (bits >> BEGIN) & ((1ULL << SIZE) - 1);
    }

    constexpr BitField& operator=(T other) {
        T mask = ((1ULL << SIZE) - 1) << BEGIN;
        bits = (bits & ~mask) | ((other << BEGIN) & mask);
        return *this;
    }

    BitField& operator=(const BitField& other) = delete;

    static constexpr size_t SIZE = END - BEGIN + 1;
};

template<size_t BEGIN, size_t END>
using U32BitField = BitField<uint32_t, BEGIN, END>;

template<size_t BEGIN, size_t END>
using U64BitField = BitField<uint64_t, BEGIN, END>;

#endif /* __BIT_FIELD_HPP__ */

