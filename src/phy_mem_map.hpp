#ifndef __PHY_MEM_MAP_HPP__
#define __PHY_MEM_MAP_HPP__

#include <stdint.h>
#include <vector>

class PhysicalMemoryMap {
    public:
        uint32_t readWord(uint32_t addr) const;
        uint16_t readHalfword(uint32_t addr) const;
        uint8_t  readByte(uint32_t addr) const;

        void writeWord(uint32_t addr, uint32_t val);
        void writeHalfword(uint32_t addr, uint16_t val);
        void writeByte(uint32_t addr, uint8_t val);

        void registerHandler(const MemoryMapHandler& handler);
        const MemoryMapHandler& getHandler(uint32_t addr) const;
    private:
        std::vector<MemoryMapHandler> _handlers;
}

#endif /* __PHY_MEM_MAP_HPP__ */
