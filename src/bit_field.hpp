#ifndef __BIT_FIELD_HPP__
#define __BIT_FIELD_HPP__

#include <cstdint>
#include <cstddef>

template <typename T, size_t BEGIN, size_t END>
struct BitField {
    T& bits;

    BitField(T& value): bits(value) {}
    BitField(const BitField& other) = default;
    // BitField(BitField&& other) = default;

    operator T() const {
        return (bits >> BEGIN) & ((1ULL << _size) - 1);
    }

    BitField& operator=(T other) {
        T mask = ((1ULL << _size) - 1) << BEGIN;
        bits = (bits & ~mask) | ((other << BEGIN) & mask);
        return *this;
    }

    BitField& operator=(const BitField& other) {
        this->bits = other.bits;
        return *this;
    }

    static constexpr size_t _size = END - BEGIN + 1;
};

template<size_t BEGIN, size_t END>
using U32BitField = BitField<uint32_t, BEGIN, END>;

template<size_t BEGIN, size_t END>
using U64BitField = BitField<uint64_t, BEGIN, END>;

#endif /* __BIT_FIELD_HPP__ */

