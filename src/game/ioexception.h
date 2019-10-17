#ifndef IOEXCEPTION_H
#define IOEXCEPTION_H

#include <stdexcept>
#include <string>

class IOException : public std::runtime_error
{
public:
    IOException(const std::string &message)
        : std::runtime_error(message) { };
};

#endif // IOEXCEPTION_H
