#ifndef __MEM_MAP_MANAGER_HPP__
#define __MEM_MAP_MANAGER_HPP__

#include <stdint.h>
#include <vector>
#include "mem_map_handler.hpp"

class MemoryMapManager {
    public:
        uint32_t readWord(uint32_t addr) throw ();
        uint16_t readHalfword(uint32_t addr) throw ();
        uint8_t  readByte(uint32_t addr) throw ();

        void writeWord(uint32_t addr, uint32_t val) throw ();
        void writeHalfword(uint32_t addr, uint16_t val) throw ();
        void writeByte(uint32_t addr, uint8_t val) throw ();

        void registerHandler(MemoryMapHandler& handler) throw ();
        MemoryMapHandler& getHandler(uint32_t addr) throw ();
    private:
        std::vector<MemoryMapHandler*> _handlers;
};

#endif /* __MEM_MAP_MANAGER_HPP__ */
