#include <string>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <ncurses.h>
#include "memory.hpp"
#include "mem_map_manager.hpp"
#include "hart.hpp"
#include "emulator_exception.hpp"

static bool isRunning = true;

void putCharCallback(char c) {
    if(c == '\n') {
        std::cout << '\r';
    }
    std::cout << c << std::flush;
}

char getCharCallback() {
    char c = getch();
    if(c == EOF) {
        return -1;
    }
    return c;
}

void shutdownCallback() {
    isRunning = false;
}

bool loadMemory(std::string fileName, uint32_t startAddr, LEMemory &memory) {
    std::ifstream file(fileName, std::ios::in | std::ios::binary | std::ios::ate);
    if(!file) {
        return false;
    }

    uint32_t nBytes = file.tellg();
    uint32_t lastAddr = startAddr + nBytes;

    if(startAddr < memory.getStartAddr() || nBytes >= memory.getSize()) {
        return false;
    }

    file.seekg(0, std::ios::beg);
    for(uint32_t addr = startAddr; addr < lastAddr; ++addr) {
        uint32_t byte = file.get();
        if(file.eof()) {
            return false;
        }
        memory.writeByte(addr, byte);
    }

    return true;
}

void initCurses() {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
}

int main(int argc, const char *argv[]) {
    const uint32_t timebaseFreq = 10000000;
    std::string fileName;
    MemoryMapManager mmap;

    // 0x8000000 = 134 MB of memory
    LEMemory memory(0x80000000, 0x8000000);
    mmap.registerHandler(memory);

    fileName = "linux/Image";
    if(!loadMemory(fileName, 0x80400000, memory)) {
        std::cout << "Trouble reading file " << fileName << "!" << std::endl;
        return -1;
    }

    fileName = "linux/initramfs.cpio.gz";
    if(!loadMemory(fileName, 0x84400000, memory)) {
        std::cout << "Trouble reading file " << fileName << "!" << std::endl;
        return -1;
    }

    fileName = "linux/emu.dtb";
    if(!loadMemory(fileName, 0x87000000, memory)) {
        std::cout << "Trouble reading file " << fileName << "!" << std::endl;
        return -1;
    }

    HartRV32 hart(0x80400000, mmap, timebaseFreq, putCharCallback, getCharCallback, shutdownCallback);

    // put DTB address in a1 for kernel
    hart.getRegisterFile().setRegister(11, 0x87000000);

    initCurses();

    while(isRunning) {
        try {
            hart.stepInstruction();
        } catch(EmulatorException &ee) {
            std::cout << ee.what() << std::endl;
            isRunning = false;
        }
    }

    endwin();
}

