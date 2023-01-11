#include "mem_map_manager.hpp"
#include "mem_map_handler.hpp"
#include "emulator_exception.hpp"

uint32_t MemoryMapManager::readWord(uint32_t addr) throw () {
    return getHandler(addr).readWord(addr);
}

uint16_t MemoryMapManager::readHalfword(uint32_t addr) throw () {
    return getHandler(addr).readHalfword(addr);
}

uint8_t MemoryMapManager::readByte(uint32_t addr) throw () {
    return getHandler(addr).readByte(addr);
}

void MemoryMapManager::writeWord(uint32_t addr, uint32_t val) throw () {
    getHandler(addr).writeWord(addr, val);
}

void MemoryMapManager::writeHalfword(uint32_t addr, uint16_t val) throw () {
    getHandler(addr).writeHalfword(addr, val);
}

void MemoryMapManager::writeByte(uint32_t addr, uint8_t val) throw () {
    getHandler(addr).writeByte(addr, val);
}

MemoryMapHandler& MemoryMapManager::getHandler(uint32_t addr) throw () {
    for(auto *handler : _handlers) {
        if(addr >= handler->getStartAddr() && addr < handler->getStartAddr() + handler->getSize()) {
            return *handler;
        }
    }

    throw EmulatorException("No registered handler");
}

void MemoryMapManager::registerHandler(MemoryMapHandler& handler) throw () {
    _handlers.push_back(&handler);
}

