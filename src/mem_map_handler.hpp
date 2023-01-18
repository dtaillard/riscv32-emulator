#ifndef __MEM_MAP_HANDLER_HPP__
#define __MEM_MAP_HANDLER_HPP__

#include <cstdint>

class MemoryMapHandler {
    public:
        MemoryMapHandler(uint32_t baseAddr, uint32_t size) : baseAddr(baseAddr), size(size) {}
        virtual ~MemoryMapHandler() = default;

        virtual uint8_t readByte(uint32_t addr) const = 0;
        virtual void writeByte(uint32_t addr, uint8_t val) = 0;

        virtual uint32_t getBaseAddr() const = 0;
        virtual uint32_t getSize() const = 0;
    protected:
        const uint32_t baseAddr;
        const uint32_t size;
};

#endif /* __MEM_MAP_HANDLER_HPP__ */
