#include "phy_mem_map.hpp"
#include "emulator_exception.hpp"

uint32_t PhysicalMemoryMap::readWord(uint32_t addr) const throw () {
    return getHandler(addr).readWord(addr);
}

uint16_t PhysicalMemoryMap::readHalfword(uint32_t addr) const throw () {
    return getHandler(addr).readHalfword(addr);
}

uint8_t PhysicalMemoryMap::readByte(uint32_t addr) const throw () {
    return getHandler(addr).readByte(addr);
}

void PhysicalMemoryMap::writeWord(uint32_t addr, uint32_t val) throw () {
    getHandler(addr).writeWord(addr, val);
}

void PhysicalMemoryMap::writeHalfword(uint32_t addr, uint16_t val) throw () {
    getHandler(addr).writeHalfword(addr, val);
}

void PhysicalMemoryMap::writeByte(uint32_t addr, uint8_t val) throw () {
    getHandler(addr).writeByte(addr, val);
}

const MemoryMapHandler& PhysicalMemoryMap::getHandler(uint32_t addr) const throw () {
    for(const auto& handler : _memoryHandlers) {
        if(addr > handler.getStartAddr() && addr < handler.getStartAddr() + handler.getSize()) {
            return handler;
        }
    }

    throw EmulatorException("No registered handler");
}

void PhysicalMemoryMap::registerHandler(const MemoryMapHandler& handler) const throw () {
    _handlers.push_back(handler);
}

