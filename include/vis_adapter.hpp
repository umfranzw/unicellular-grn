#ifndef _VIS_ADAPTER_HPP
#define _VIS_ADAPTER_HPP

#include <sqlite3.h>
#include <vector>
#include <string>
#include <zmq.hpp>

#define SCRIPT_PATH "/home/wayne/Documents/school/thesis/unicellular-grn/vis/test.py"
#define CHUNK_SIZE 20

using namespace std;

typedef enum STATES {
    INITIAL,
    PARAMS,
    STEP
} STATES;

class VisAdapter {
public:
    VisAdapter(sqlite3 *db);
    ~VisAdapter();
    void listen();
    void select(string sql, vector<string> params, vector<char> types);
    char *recv_string(zmq::socket_t *socket);
    double recv_double(zmq::socket_t *socket);
    int recv_int(zmq::socket_t *socket);

private:
    void run_script();
    int read_stream(int fd, char **buf);

    sqlite3 *db;
    int state;
    sqlite3_stmt *stmt;
};

#endif
