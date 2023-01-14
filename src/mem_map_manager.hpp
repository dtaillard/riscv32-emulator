#ifndef __MEM_MAP_MANAGER_HPP__
#define __MEM_MAP_MANAGER_HPP__

#include <stdint.h>
#include <vector>
#include "mem_map_handler.hpp"

class MemoryMapManager {
    public:
        uint32_t readWord(uint32_t addr);
        uint16_t readHalfword(uint32_t addr);
        uint8_t  readByte(uint32_t addr);

        void writeWord(uint32_t addr, uint32_t val);
        void writeHalfword(uint32_t addr, uint16_t val);
        void writeByte(uint32_t addr, uint8_t val);

        void registerHandler(MemoryMapHandler& handler);
        MemoryMapHandler& getHandler(uint32_t addr);
    private:
        std::vector<MemoryMapHandler*> handlers;
};

#endif /* __MEM_MAP_MANAGER_HPP__ */
