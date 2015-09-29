// compress.cpp
#include "compress.h"
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
using namespace std;
using namespace agenda;

// pipebuf
class pipebuf : public streambuf
{
public:
    pipebuf();
    ~pipebuf();

    int get_read_fd()
    { return _pfd[0]; }
    void close_write_fd();
    void close_read_fd();
private:
    static const ptrdiff_t BUFSIZE = 4096;

    virtual int_type overflow(int_type);
    virtual int sync();

    // disallow copying
    pipebuf(const pipebuf&);
    pipebuf& operator =(const pipebuf&);

    int _pfd[2];
    char _buffer[BUFSIZE+1];
};

pipebuf::pipebuf()
{
    if (pipe(_pfd) == -1)
        throw compression_exception("fail pipe()");
    setp(_buffer,_buffer+BUFSIZE);
}
pipebuf::~pipebuf()
{
    for (short i = 0;i < 2;++i)
        if (_pfd[i] != -1)
            close(_pfd[i]);
}
void pipebuf::close_write_fd()
{
    if (_pfd[1] != -1) {
        close(_pfd[1]);
        _pfd[1] = -1;
    }
}
void pipebuf::close_read_fd()
{
    if (_pfd[0] != -1) {
        close(_pfd[0]);
        _pfd[0] = -1;
    }
}
streambuf::int_type pipebuf::overflow(streambuf::int_type ch)
{
    if (ch == traits_type::eof())
        return traits_type::eof();
    // write all data in streambuf to pipe
    char* base = pbase(), *e = pptr();
    ptrdiff_t n = e - base;
    *e = ch; // guarenteed to be present at end of buffer
    if (write(_pfd[1],base,n+1) == -1)
        throw compression_exception("fail write()");
    pbump(-n);
    return ch;
}
int pipebuf::sync()
{
    // write all data in streambuf to pipe
    char* base = pbase(), *e = pptr();
    ptrdiff_t n = e - base;
    if (n > 0) {
        if (write(_pfd[1],base,n) == -1)
            throw compression_exception("fail write()");
        pbump(-n);
    }
    return 0;
}

static string compressed_file_version(const string& fileName)
{
    int vNo = 1;
    while (true) {
        char buffer[32];
        string attempt(fileName);
        snprintf(buffer,sizeof(buffer),"%d",vNo);
        attempt += ".";
        attempt += buffer;
        attempt += ".gz";

        if (access(attempt.c_str(),F_OK) == -1) {
            if (errno == ENOENT)
                return attempt;
            throw compression_exception("fail access() for unknown reason");
        }

        ++vNo;
    }

    throw compression_exception("exception in compressed_file_version()");
}

// compressed_ostream
compressed_ostream::compressed_ostream(const string& fileName)
    : buf(new pipebuf), pid(-1)
{
    // open output file
    int out = open(compressed_file_version(fileName).c_str(),O_CREAT|O_TRUNC|O_WRONLY,0666);
    if (out == -1) {
        delete buf;
        throw compression_exception("couldn't open output file");
    }

    // fork into new process
    pid_t proc = fork();
    if (proc == -1) {
        delete buf;
        throw compression_exception("fail fork()");
    }
    if (proc == 0) {
        static const char* args[] = {
            "gzip", "-c", "-", NULL
        };

        // redirect stdin to read end of pipe; redirect stdout to output file
        if (dup2(buf->get_read_fd(),STDIN_FILENO) == -1 || dup2(out,STDOUT_FILENO) == -1)
            throw compression_exception("failed to duplicate standard I/O");
        // close output file descriptor and the pipe
        close(out);
        buf->close_write_fd();
        buf->close_read_fd();

        // execute gzip
        if (execvp("gzip",(char*const*)args) == -1)
            _exit(1);
        // control no longer in this program
    }

    // in parent: close output file, close read end of pipe
    close(out);
    buf->close_read_fd();

    pid = proc;
    rdbuf(buf); // set stream buffer for ostream
}
compressed_ostream::~compressed_ostream()
{
    delete buf;
}
void compressed_ostream::wait()
{
    // flush stream buffer and wait on the child process
    int status = -1;
    pid_t ret;

    buf->pubsync();
    buf->close_write_fd();
    ret = waitpid(pid,&status,0);

    if (ret == -1 || status != 0)
        throw compression_exception("gzip failed");
}
