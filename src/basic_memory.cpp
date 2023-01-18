#include "basic_memory.hpp"
#include "emulator_exception.hpp"
#include <cassert>

BasicMemory::BasicMemory(uint32_t baseAddr, uint32_t size) : MemoryMapHandler(baseAddr, size), memoryArray(std::make_unique<uint8_t[]>(size)) {
}

uint8_t BasicMemory::readByte(uint32_t addr) const {
    assert(addr >= baseAddr && addr < baseAddr + size);
    return memoryArray[addr - baseAddr];
}

void BasicMemory::writeByte(uint32_t addr, uint8_t val) {
    assert(addr >= baseAddr && addr < baseAddr + size);
    memoryArray[addr - baseAddr] = val;
}

uint32_t BasicMemory::getBaseAddr() const {
    return baseAddr;
}

uint32_t BasicMemory::getSize() const {
    return size;
}

