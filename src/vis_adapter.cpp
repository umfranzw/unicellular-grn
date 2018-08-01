#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include "vis_adapter.hpp"

VisAdapter::VisAdapter(sqlite3 *db) {
    this->db = db;
}

void VisAdapter::listen() {
    cout << "Starting server" << endl;
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PAIR);
    socket.bind("ipc://pipe");

    this->run_script();

    bool done = false;
    while (!done) {
        //wait for client to send command
        char *cmd = this->recv_string(&socket);
        cout << "Server received cmd: " << cmd << endl;
        if (strcmp(cmd, "stmt") == 0) {
            //receive sql
            char *sql = this->recv_string(&socket);
            cout << "Server received sql: " << endl << sql << endl;
            sqlite3_prepare_v2(this->db, sql, strlen(sql) + 1, &this->stmt, NULL);
            free(sql);
        }

        else if (strcmp(cmd, "bind_int") == 0) {
            //receive param index
            int index = this->recv_int(&socket);
            cout << "Server received index: " << index << endl;
            int val = this->recv_int(&socket);
            cout << "Server received value: " << val << endl;
            sqlite3_bind_int(this->stmt, index, val);
        }
        
        else if (strcmp(cmd, "bind_double") == 0) {
            int index = this->recv_int(&socket);
            cout << "Server received index: " << index << endl;
            double val = this->recv_double(&socket);
            cout << "Server received value: " << val << endl;
            sqlite3_bind_double(this->stmt, index, val);
        }

        else if (strcmp(cmd, "bind_text") == 0) {
            int index = this->recv_int(&socket);
            cout << "Server received index: " << index << endl;
            char *val = this->recv_string(&socket);
            cout << "Server received value: " << val << endl;
            sqlite3_bind_text(this->stmt, index, val, strlen(val), SQLITE_TRANSIENT);
            free(val); //safe since TRANSIENT param causes SQLite to copy the string above
        }

        else if (strcmp(cmd, "step") == 0) {
            int rc = sqlite3_step(this->stmt);
            if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
                cerr << "sqlite error in vis_adapter: " << rc << endl;
                cerr << sqlite3_errmsg(this->db) << endl;
            }
            int resp_val = (rc == SQLITE_ROW); //send 1 if row, 0 if done or error
            zmq::message_t resp(sizeof(int));
            memcpy(resp.data(), &resp_val, sizeof(int));
            socket.send(resp);
            cout << "Server sent response code: " << resp_val << endl;
        }

        else if (strcmp(cmd, "column_int") == 0) {
            //receive index
            int index = this->recv_int(&socket);
            cout << "Server received index: " << index << endl;
            
            int val = sqlite3_column_int(this->stmt, index);
            zmq::message_t resp(sizeof(int));
            memcpy(resp.data(), &val, sizeof(int));
            socket.send(resp);
        }

        else if (strcmp(cmd, "column_double") == 0) {
            int index = this->recv_int(&socket);
            cout << "Server received index: " << index << endl;

            double val = sqlite3_column_double(this->stmt, index);
            zmq::message_t resp(sizeof(double));
            memcpy(resp.data(), &val, sizeof(double));
            socket.send(resp);
        }

        else if (strcmp(cmd, "column_text") == 0) {
            int index = this->recv_int(&socket);
            cout << "Server received index: " << index << endl;

            char *val = (char *) sqlite3_column_text(this->stmt, index);
            int len = strlen(val);
            zmq::message_t resp(len);
            memcpy(resp.data(), val, len); //note: don't copy the '\0'
            socket.send(resp);
        }

        else if (strcmp(cmd, "finalize") == 0) {
            sqlite3_finalize(this->stmt);
            cout << "Server finalized statement" << endl;
        }

        else if (strcmp(cmd, "exit") == 0) {
            done = true;
            cout << "Server exited" << endl;
        }
        
        free(cmd);
    }

    remove("./pipe");
}

//note: must free returned pointer
char *VisAdapter::recv_string(zmq::socket_t *socket) {
    zmq::message_t req;
    socket->recv(&req);
    size_t size = req.size();
    char *data = (char *) malloc(sizeof(char) * (size + 1));
    memcpy(data, req.data(), size);
    data[size] = '\0';

    return data;
}

double VisAdapter::recv_double(zmq::socket_t *socket) {
    zmq::message_t req;
    socket->recv(&req);
    size_t size = req.size();
    double data = 0;
    memcpy(&data, req.data(), size);

    return data;
}

int VisAdapter::recv_int(zmq::socket_t *socket) {
    zmq::message_t req;
    socket->recv(&req);
    size_t size = req.size();
    int data = 0; //note: python may send less than 4 bytes, so we must zero it out first
    memcpy(&data, req.data(), size);

    return data;
}

VisAdapter::~VisAdapter() {
}

void VisAdapter::run_script() {
    // int output_pipe[2]; //stdin, stdout
    // int error_pipe[2];
    pid_t child_pid;
    ////int status;
    
    // pipe(output_pipe);
    // pipe(error_pipe);

    //note: CPU/core affinity will be inherited by child processes by default
    child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        exit(1);
    }
    
    else if (child_pid == 0) {
        //duplicate the output side of the pipe to stdout
        //loop is in case of interruption by signal
        // while ((dup2(output_pipe[1], STDOUT_FILENO) == -1) && (errno == EINTR));
        // close(output_pipe[1]);
        // close(output_pipe[0]);
        
        // while ((dup2(error_pipe[1], STDERR_FILENO) == -1) && (errno == EINTR));
        // close(error_pipe[1]);
        // close(error_pipe[0]);
        
        execlp(SCRIPT_PATH, "", NULL);
        perror("execlp");
        exit(1);
    }
    // close(output_pipe[1]);
    // close(error_pipe[1]);

    ////waitpid(child_pid, &status, 0);

    // char *out_buf;
    // int count = this->read_stream(output_pipe[0], &out_buf);
    // if (count > 0) {
    //     cout << string(out_buf) << endl;
    // }

    // char *err_buf;
    // count = this->read_stream(error_pipe[0], &err_buf);
    // if (count > 0) {
    //     cerr << string(err_buf) << endl;
    // }

    // //clean up
    // free(out_buf);
    // free(err_buf);
    // close(output_pipe[0]);
    // close(error_pipe[0]);
}

//note: count does not include null terminator
int VisAdapter::read_stream(int fd, char **buf) {
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
