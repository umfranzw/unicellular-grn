import sqlite3

FILENAME = '../../data/dbs/run0.db'

#just write a custom spin button, man!!!
class Db:
    def __init__(self):
        self.conn = sqlite3.connect(FILENAME)
        self.cur = self.conn.cursor()

    def close(self):
        self.conn.close()
