#ifndef __EMULATOR_EXCEPTION_HPP__ 
#define __EMULATOR_EXCEPTION_HPP__

#include <string>
#include <sstream>
#include <exception>

class EmulatorException: public std::exception {
    public:
        EmulatorException(const std::string& message);
        const std::string& what();
    private:
        std::string _message;
};

#endif /* __EMULATOR_EXCEPTION_HPP__ */
