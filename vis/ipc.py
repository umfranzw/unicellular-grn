import zmq
import sys
import struct

SOCKET_URI = 'ipc:///tmp/pipe'

class Ipc():
    def __init__(self):
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.PAIR);
        self.socket.connect(SOCKET_URI)

    def select(self, sql, params, result_col_types):
        results = []

        self.socket.send(b'stmt')
        self.socket.send(bytes(sql, 'utf-8'))

        bind_index = 1
        for param in params:
            if type(param) is int:
                self.socket.send(b'bind_int')
                ba = bytearray(struct.pack('i', bind_index))
                self.socket.send(ba)
                ba = bytearray(struct.pack('i', param))
                self.socket.send(ba)

            elif type(param) is float:
                self.socket.send(b'bind_double')
                ba = bytearray(struct.pack('i', bind_index))
                self.socket.send(ba)
                ba = bytearray(struct.pack('d', param))
                self.socket.send(ba)

            elif type(param) is str:
                self.socket.send(b'bind_text')
                ba = bytearray(struct.pack('i', bind_index))
                self.socket.send(ba)
                self.socket.send(bytes(param)) #note the lack of sqare brackets!

            bind_index += 1

        #get first row
        self.socket.send(b'step')
        status_bytes = self.socket.recv()
        status = int.from_bytes(status_bytes, sys.byteorder)
        while status == 1:
            row = []
            bind_index = 0
            for col_type in result_col_types:
                if col_type == int:
                    self.socket.send(b'column_int')
                    ba = bytearray(struct.pack('i', bind_index))
                    self.socket.send(ba)

                    data_bytes = self.socket.recv()
                    data = struct.unpack('i', data_bytes)[0]
                    row.append(data)

                elif col_type == float:
                    self.socket.send(b'column_double')
                    ba = bytearray(struct.pack('i', bind_index))
                    self.socket.send(ba)

                    data_bytes = self.socket.recv()
                    data = struct.unpack('d', data_bytes)[0]
                    row.append(data)

                elif col_type == str:
                    self.socket.send(b'column_text')
                    ba = bytearray(struct.pack('i', bind_index))
                    self.socket.send(ba)

                    data_bytes = self.socket.recv()
                    data = data_bytes.decode()
                    row.append(data)

                bind_index += 1

            results.append(row)
            #get next row
            self.socket.send(b'step')
            status_bytes = self.socket.recv()
            status = int.from_bytes(status_bytes, sys.byteorder)

        self.socket.send(b'finalize')

        return results

    def close(self):
        self.socket.send(b'exit')


