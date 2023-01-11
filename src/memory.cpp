#include "memory.hpp"
#include "emulator_exception.hpp"

LEMemory::LEMemory(uint32_t baseAddr, uint32_t size) : _size(size), _baseAddr(baseAddr) {
    _memoryArray = new uint8_t[_size];    
}

LEMemory::~LEMemory() {
    delete[] _memoryArray;
}

uint32_t LEMemory::readWord(uint32_t addr) const {
    uint16_t half0 = readHalfword(addr);
    uint16_t half1 = readHalfword(addr + 2);

    return half0 | (half1 << 16);
}


uint16_t LEMemory::readHalfword(uint32_t addr) const {
    uint8_t byte0 = readByte(addr);
    uint8_t byte1 = readByte(addr + 1);

    return byte0 | (byte1 << 8);
}

uint8_t LEMemory::readByte(uint32_t addr) const {
    if(addr >= _baseAddr + _size) {
        throw EmulatorException("Write exceeded memory size");
    }
    return _memoryArray[addr - _baseAddr];
}

void LEMemory::writeWord(uint32_t addr, uint32_t val) {
    uint16_t half0 = val & 0xFFFF;
    uint16_t half1 = (val >> 16) & 0xFFFF;

    writeHalfword(addr, half0);
    writeHalfword(addr + 2, half1);
}

void LEMemory::writeHalfword(uint32_t addr, uint16_t val) {
    uint8_t byte0 = val & 0xFF;
    uint8_t byte1 = (val >> 8) & 0xFF;

    writeByte(addr, byte0);
    writeByte(addr + 1, byte1);
}

void LEMemory::writeByte(uint32_t addr, uint8_t val) {
    if(addr >= _baseAddr + _size) {
        throw EmulatorException("Write exceeded memory size");
    }
    _memoryArray[addr - _baseAddr] = val;
}

uint32_t LEMemory::getStartAddr() {
    return _baseAddr;
}

uint32_t LEMemory::getSize() {
    return _size;
}

