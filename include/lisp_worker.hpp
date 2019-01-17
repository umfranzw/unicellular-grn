#ifndef _LISP_WORKER_HPP
#define _LISP_WORKER_HPP

#include <string>

using namespace std;

#define CHUNK_SIZE 20

class LispWorker {
public:
    LispWorker(string lisp_code);
    bool run();
    string get_output();
    
private:
    string lisp_code;
    string output;

    string trim_newline(char *buf, int len);
    int read_stream(int fd, char **buf);
};

#endif
