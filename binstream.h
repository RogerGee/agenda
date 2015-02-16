/* binstream.h */
#ifndef BINSTREAM_H
#define BINSTREAM_H
#include <cstdint>
#include <streambuf>
#include <string>

namespace agenda
{
    // define errors we care about
    enum file_does_not_exist { };

    class binfilebuf : public std::streambuf
    {
    protected:
        binfilebuf(const char* name,int flags);
        ~binfilebuf();

        virtual int sync();
    private:
        static const std::ptrdiff_t BUFSIZE = 4096;

        // input
        virtual int_type underflow();

        // output
        virtual int_type overflow(int_type);

        int _fd;
        char _buffer[BUFSIZE+1];
    };

    class binfilebuf_in : public binfilebuf
    {
    public:
        binfilebuf_in(const char* file);

        uint16_t get_uint16();
        uint32_t get_uint32();
        std::string get_string();
    };
    class binfilebuf_out : public binfilebuf
    {
    public:
        binfilebuf_out(const char* file);
        ~binfilebuf_out();

        void put_uint16(uint16_t value);
        void put_uint32(uint32_t value);
        void put_string(const std::string& value);
    };

    class binreader
    {
    public:
        binreader(const char* file)
            : _buf(file) {}

        binreader& operator >>(uint16_t& t)
        { return t=_buf.get_uint16(), *this; }
        binreader& operator >>(uint32_t& t)
        { return t=_buf.get_uint32(), *this; }
        binreader& operator >>(std::string& s)
        { return s=_buf.get_string(), *this; }

        bool eof() const
        { return const_cast<binfilebuf_in&>(_buf).sgetc() == binfilebuf_in::traits_type::eof(); }
    private:
        binfilebuf_in _buf;
    };

    class binwriter
    {
    public:
        binwriter(const char* file)
            : _buf(file) {}

        binwriter& operator <<(uint16_t t)
        { return _buf.put_uint16(t), *this; }
        binwriter& operator <<(uint32_t t)
        { return _buf.put_uint32(t), *this; }
        binwriter& operator <<(const std::string& s)
        { return _buf.put_string(s), *this; }
    private:
        binfilebuf_out _buf;
    };
}

#endif
