#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <string>
#include "lisp_worker.hpp"

using namespace std;

LispWorker::LispWorker(string lisp_code) {
    this->lisp_code = lisp_code;
}

bool LispWorker::run() {
    bool success = false;

    int output_pipe[2]; //stdin, stdout
    int error_pipe[2];
    pid_t child_pid;
    int status;
    
    pipe(output_pipe);
    pipe(error_pipe);

    //note: CPU/core affinity will be inherited by child processes by default
    child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        exit(1);
    }
    
    else if (child_pid == 0) {
        //duplicate the output side of the pipe to stdout
        //loop is in case of interruption by signal
        while ((dup2(output_pipe[1], STDOUT_FILENO) == -1) && (errno == EINTR));
        close(output_pipe[1]);
        close(output_pipe[0]);
        
        while ((dup2(error_pipe[1], STDERR_FILENO) == -1) && (errno == EINTR));
        close(error_pipe[1]);
        close(error_pipe[0]);
        
        execlp("clisp", "-q", "-q", "-norc", "-x", this->lisp_code.c_str(), NULL);
        perror("execlp");
        exit(1);
    }
    close(output_pipe[1]);
    close(error_pipe[1]);
    
    waitpid(child_pid, &status, 0);

    char *out_buf;
    int count = this->read_stream(output_pipe[0], &out_buf);
    if (count > 0) {
        success = true;
        this->output = string(out_buf);
    }

    char *err_buf;
    count = this->read_stream(error_pipe[0], &err_buf);
    if (count > 0) {
        this->output = string(err_buf);
    }

    //clean up
    free(out_buf);
    free(err_buf);
    close(output_pipe[0]);
    close(error_pipe[0]);

    return success;
}

string LispWorker::get_output() {
    return this->output;
}

//note: count does not include null terminator
int LispWorker::read_stream(int fd, char **buf) {
    *buf = (char *) malloc((CHUNK_SIZE + 1) * sizeof(char)); //reserve one slot for '\0'
    int buf_size = CHUNK_SIZE;

    int count = 0;
    ssize_t bytes = read(fd, *buf + count, 1);
    while (bytes > 0) {
        count++;
        if (count == buf_size) {
            char *new_ptr = (char *) realloc(*buf, buf_size + CHUNK_SIZE);
            if (new_ptr == NULL) {
                perror("realloc");
                free(*buf);
                exit(1);
            }
            else {
                *buf = new_ptr;
            }
            buf_size += CHUNK_SIZE;
        }
        bytes = read(fd, *buf + count, 1);
    }
    (*buf)[count] = '\0';

    return count;
}
