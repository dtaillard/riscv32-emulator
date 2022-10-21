#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdint>

#define SIGN_EXTEND(n, b) ((-(n >> (b - 1)) << b) | n)

union RTypeFormat {
    struct {
        uint32_t opcode : 7;
        uint32_t rd : 5;
        uint32_t funct3 : 3;
        uint32_t rs1 : 5;
        uint32_t rs2 : 5;
        uint32_t funct7 : 7;
    } fields;
    uint32_t value;
};


int main(int argc, const char *argv[]) {
    uint32_t test = 0x12345678;
    RTypeFormat fmt;
    fmt.value = test;
    std::cout << "funct7: " << fmt.fields.funct7 << std::endl;

    // 0001 0010

    uint32_t a = 0x1F;   // 0001 1111
    uint32_t c = SIGN_EXTEND(a, 5);
    uint32_t five = 5;
    std::cout << "-1 + 5: " << (int32_t)(c + five) << std::endl;
    std::cout << "Sign extended value: " << c << std::endl;
    uint32_t b = SIGN_EXTEND(a, 6);
    std::cout << "Sign extended value: " << b << std::endl;
    uint8_t foo = a;
}

