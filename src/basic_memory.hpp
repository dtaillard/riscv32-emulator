#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

#include <cstdint>
#include <memory>
#include "mem_map_handler.hpp"

class BasicMemory: public MemoryMapHandler {
    public:
        BasicMemory(uint32_t baseAddr, uint32_t size);

        uint8_t readByte(uint32_t addr) const;
        void writeByte(uint32_t addr, uint8_t val);

        uint32_t getBaseAddr() const;
        uint32_t getSize() const;
     private:
        std::unique_ptr<uint8_t[]> memoryArray;
};

#endif /*  __MEMORY_HPP__ */
