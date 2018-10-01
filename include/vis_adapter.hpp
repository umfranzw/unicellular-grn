#ifndef _VIS_ADAPTER_HPP
#define _VIS_ADAPTER_HPP

#if HAVE_ZMQ
#include "runs.hpp"
#include <sqlite3.h>
#include <vector>
#include <string>
#include <zmq.hpp>

#define SCRIPT_PATH "vis/vis.py"
#define CHUNK_SIZE 20
#define SOCKET_URI "ipc:///tmp/pipe"

using namespace std;

class VisAdapter {
public:
    VisAdapter(Run *run, sqlite3 *db);
    ~VisAdapter();
    void listen();
    void select(string sql, vector<string> params, vector<char> types);
    char *recv_string(zmq::socket_t *socket);
    double recv_double(zmq::socket_t *socket);
    int recv_int(zmq::socket_t *socket);

private:
    void run_script();
    int read_stream(int fd, char **buf);

    Run *run;
    sqlite3 *db;
    sqlite3_stmt *stmt;
};
#endif
#endif
