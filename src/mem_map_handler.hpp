#ifndef __MEM_MAP_HANDLER_HPP__
#define __MEM_MAP_HANDLER_HPP__

#include <cstdint>

class MemoryMapHandler {
    public:
        virtual ~MemoryMapHandler() = default;

        virtual void getStartAddr() = 0;
        virtual void getSize() = 0;

        virtual uint32_t readWord(uint32_t addr) const = 0;
        virtual uint16_t readHalfword(uint32_t addr) const = 0;
        virtual uint8_t  readByte(uint32_t addr) const = 0;

        virtual void writeWord(uint32_t addr, uint32_t val) = 0;
        virtual void writeHalfword(uint32_t addr, uint16_t val) = 0;
        virtual void writeByte(uint32_t addr, uint8_t val) = 0;
};

#endif /* __MEM_MAP_HANDLER_HPP__ */
