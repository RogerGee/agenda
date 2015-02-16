/* binstream.cpp */
#include "binstream.h"
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
using namespace std;
using namespace agenda;

// binfilebuf
binfilebuf::binfilebuf(const char* name,int flags)
{
    char* pbuf;
    _fd = open(name,flags,0666);
    if (_fd == -1) {
        if (errno == ENOENT)
            // signal that the file does not exist
            throw file_does_not_exist(0);
        ostringstream ss;
        ss << "couldn't open agenda file '" << name << "': " << strerror(errno) << endl;
        throw runtime_error(ss.str());
    }
    pbuf = _buffer + BUFSIZE;
    setg(pbuf,pbuf,pbuf);
    setp(_buffer,pbuf);
}
binfilebuf::~binfilebuf()
{
    close(_fd);
}
binfilebuf::int_type binfilebuf::underflow()
{
    if (gptr() < egptr())
        return traits_type::to_int_type(*gptr());
    // buffer is empty; read from the device
    ssize_t i;
    i = read(_fd,_buffer,sizeof(_buffer));
    if (i == -1) {
        ostringstream ss;
        ss << "error on file read: " << strerror(errno);
        throw runtime_error(ss.str());
    }
    if (i == 0)
        return traits_type::eof();
    setg(_buffer,_buffer,_buffer+i);
    return traits_type::to_int_type(*gptr());
}
binfilebuf::int_type binfilebuf::overflow(int_type ch)
{
    if (ch == traits_type::eof())
        return traits_type::eof();
    // attempt to write all data in streambuf to file
    char* base = pbase(), *e = pptr();
    ptrdiff_t n = e - base;
    *e = ch; // safe since we allocated an extra space
    if (write(_fd,base,n) == -1) {
        ostringstream ss;
        ss << "error on file write: " << strerror(errno);
        throw runtime_error(ss.str());
    }
    pbump(-n);
    return ch;
}
int binfilebuf::sync()
{
    // attempt to write all data in streambuf to file
    char* base = pbase(), *e = pptr();
    ptrdiff_t n = e - base;
    if (write(_fd,base,n) == -1) {
        ostringstream ss;
        ss << "error on file write: " << strerror(errno);
        throw runtime_error(ss.str());
    }
    pbump(-n);
    return 0;
}

// binfilebuf_in
binfilebuf_in::binfilebuf_in(const char* file)
    : binfilebuf(file,O_RDONLY)
{
}
uint16_t binfilebuf_in::get_uint16()
{
    uint8_t bytes[2];
    bytes[0] = sbumpc();
    bytes[1] = sbumpc();
    return (uint16_t(bytes[1])<<8) | bytes[0];
}
uint32_t binfilebuf_in::get_uint32()
{
    uint8_t bytes[4];
    bytes[0] = sbumpc();
    bytes[1] = sbumpc();
    bytes[2] = sbumpc();
    bytes[3] = sbumpc();
    return (uint32_t(bytes[3])<<24) | (uint32_t(bytes[2]<<16)) | (uint32_t(bytes[1])<<8) | bytes[0];
}
string binfilebuf_in::get_string()
{
    string str;
    while (true) {
        char c;
        c = sbumpc();
        if (c == 0)
            break;
        str.push_back(c);
    }
    return str;
}

// binfilebuf_out
binfilebuf_out::binfilebuf_out(const char* file)
    : binfilebuf(file,O_WRONLY|O_CREAT|O_TRUNC)
{
}
binfilebuf_out::~binfilebuf_out()
{
    // make sure all the bytes get written to the file
    sync();
}
void binfilebuf_out::put_uint16(uint16_t value)
{
    sputc(value&0xff);
    sputc((value>>8) & 0xff);
}
void binfilebuf_out::put_uint32(uint32_t value)
{
    sputc(value&0xff);
    sputc((value>>8) & 0xff);
    sputc((value>>16) & 0xff);
    sputc((value>>24) & 0xff);
}
void binfilebuf_out::put_string(const string& value)
{
    for (size_t i = 0;i < value.length();++i)
        sputc(value[i]);
    sputc(0);
}
