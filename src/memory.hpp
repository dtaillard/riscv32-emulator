#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

#include <cstdint>
#include "mem_map_handler.hpp"

class LEMemory: public MemoryMapHandler {
    public:
        LEMemory(uint32_t baseAddr, uint32_t size);
        virtual ~LEMemory();

        uint32_t readWord(uint32_t addr) const;
        uint16_t readHalfword(uint32_t addr) const;
        uint8_t  readByte(uint32_t addr) const;

        void writeWord(uint32_t addr, uint32_t val);
        void writeHalfword(uint32_t addr, uint16_t val);
        void writeByte(uint32_t addr, uint8_t val);

        uint32_t getStartAddr();
        uint32_t getSize();
     private:
        uint32_t _size;
        uint32_t _baseAddr;
        uint8_t *_memoryArray;
};

#endif /*  __MEMORY_HPP__ */
