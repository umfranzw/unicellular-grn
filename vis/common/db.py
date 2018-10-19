import sqlite3

class Db:
    def __init__(self, path):
        self.conn = sqlite3.connect(path)
        self.cur = self.conn.cursor()

    def close(self):
        self.conn.close()
