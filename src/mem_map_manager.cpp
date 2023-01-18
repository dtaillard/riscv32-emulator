#include "mem_map_manager.hpp"
#include "mem_map_handler.hpp"
#include "emulator_exception.hpp"

uint32_t MemoryMapManager::readWord(uint32_t addr) {
    uint16_t half0 = readHalfword(addr);
    uint16_t half1 = readHalfword(addr + 2);

    return half0 | (half1 << 16);
}

uint16_t MemoryMapManager::readHalfword(uint32_t addr) {
    uint8_t byte0 = readByte(addr);
    uint8_t byte1 = readByte(addr + 1);

    return byte0 | (byte1 << 8);
}

uint8_t MemoryMapManager::readByte(uint32_t addr) {
    return getHandler(addr).readByte(addr);
}

void MemoryMapManager::writeWord(uint32_t addr, uint32_t val) {
    uint16_t half0 = val & 0xFFFF;
    uint16_t half1 = (val >> 16) & 0xFFFF;

    writeHalfword(addr, half0);
    writeHalfword(addr + 2, half1);
}

void MemoryMapManager::writeHalfword(uint32_t addr, uint16_t val) {
    uint8_t byte0 = val & 0xFF;
    uint8_t byte1 = (val >> 8) & 0xFF;

    writeByte(addr, byte0);
    writeByte(addr + 1, byte1);
}

void MemoryMapManager::writeByte(uint32_t addr, uint8_t val) {
    getHandler(addr).writeByte(addr, val);
}

MemoryMapHandler& MemoryMapManager::getHandler(uint32_t addr) {
    for(auto *handler : handlers) {
        uint32_t lastAddr = handler->getBaseAddr() + handler->getSize();
        if(addr >= handler->getBaseAddr() && addr < lastAddr) {
            return *handler;
        }
    }
    throw EmulatorException("No registered handler");
}

void MemoryMapManager::registerHandler(MemoryMapHandler& handler) {
    handlers.push_back(&handler);
}
