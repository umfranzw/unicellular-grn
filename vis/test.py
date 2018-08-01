#!/usr/bin/python3

import zmq
import sys
import struct

def select(socket, sql, params, result_col_types):
    results = []
    
    socket.send(b'stmt')
    print("Client sent cmd: stmt")
    socket.send(bytes(sql, 'utf-8'))
    print("Client sent sql: \n{}".format(sql))

    bind_index = 1
    for param in params:
        if type(param) is int:
            socket.send(b'bind_int')
            print("Client sent cmd: bind_int")
            socket.send(bind_index.to_bytes(4, sys.byteorder))
            print("Client sent index: {}".format(bind_index))
            socket.send(param.to_bytes(4, sys.byteorder))
            print("Client sent param: {}".format(param))
            
        elif type(param) is float:
            socket.send(b'bind_double')
            print("Client sent cmd: bind_double")
            socket.send(bind_index.to_bytes(4, sys.byteorder))
            print("Client sent index: {}".format(bind_index))
            ba = bytearray(struct.pack('d', param))
            socket.send(ba)
            print("Client sent param: {}".format(param))
            
        elif type(param) is str:
            socket.send(b'bind_text')
            print("Client sent cmd: bind_text")
            socket.send(bind_index.to_bytes(4, sys.byteorder))
            print("Client sent index: {}".format(bind_index))
            socket.send(bytes(param)) #note the lack of sqare brackets!
            print("Client sent param: {}".format(param))

        bind_index += 1

    #get first row
    socket.send(b'step')
    print('Client sent cmd: step')
    status_bytes = socket.recv()
    status = int.from_bytes(status_bytes, sys.byteorder)
    print("Client received status code: {}".format(status))
    while status == 1:
        row = []
        bind_index = 0
        for col_type in result_col_types:
            if col_type == int:
                socket.send(b'column_int')
                socket.send(bind_index.to_bytes(4, sys.byteorder))
                
                data_bytes = socket.recv()
                data = int.from_bytes(data_bytes, sys.byteorder)
                row.append(data)
                
            elif col_type == float:
                socket.send(b'column_double')
                socket.send(bind_index.to_bytes(4, sys.byteorder))
                
                data_bytes = socket.recv()
                data = float.from_bytes(data_bytes, sys.byteorder)
                row.append(data)

            elif col_type == str:
                socket.send(b'column_text')
                socket.send(bind_index.to_bytes(4, sys.byteorder))
                
                data_bytes = socket.recv()
                data = data_bytes.decode()
                row.append(data)
                
            bind_index += 1

        results.append(row)
        #get next row
        socket.send(b'step')
        status_bytes = socket.recv()
        status = int.from_bytes(status_bytes, sys.byteorder)

    return results

def main():
    print('Starting client')
    sys.stdout.flush()
    context = zmq.Context()

    socket = context.socket(zmq.PAIR);
    socket.connect('ipc://pipe')
    print('Client connected')
    sys.stdout.flush()

    sql = ('SELECT p.pid FROM grn JOIN gene g ON grn.id = g.grn_id JOIN ' +
           'gene_state gs ON gs.gene_id = g.id JOIN ' +
           'protein p ON gs.bound_protein = p.id ' + #note: gs.bound_protein is a *db* id (not a simulation id)
           'WHERE grn.ga_step = ? AND grn.pop_index = ? AND g.pos = ? AND gs.reg_step = ?;')
    params = [0, 1, 2, 0]
    result_col_types = [int]
    results = select(socket, sql, params, result_col_types)

    socket.send(b"exit");
    
    print('Results:')
    for row in results:
        print(row)


main()
