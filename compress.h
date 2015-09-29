// compress.h
#ifndef COMPRESS_H
#define COMPRESS_H
#include <iostream>

class pipebuf;
namespace agenda
{
    class compression_exception : public std::exception
    {
    public:
        compression_exception(const char* message) : _msg(message) {}
    private:
        const char* _msg;
        virtual const char* what() const throw()
        { return _msg; }
    };

    class compressed_ostream : public std::ostream
    {
    public:
        compressed_ostream(const std::string& fileName);
        ~compressed_ostream();

        void wait();
    private:
        pipebuf* buf;
        long int pid;
    };
}

#endif
