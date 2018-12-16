import sqlite3

class BestInfo():
    def __init__(self, db=None, ipc=None):
        self.db = db
        self.ipc = ipc

    def _select(self, sql, args, result_types=None):
        rs = None
        if self.db:
            self.db.cur.execute(sql, args)
            rs = self.db.cur

        else:
            rs = self.ipc.select(sql, args, result_types)

        return rs

    #returns None or (ga_step, pop_index, fitness)
    def get_fittest_grn(self):
        sql = 'SELECT f.ga_step, g.pop_index, min(f.fitness) FROM fitness f JOIN grn g ON f.pop_index = g.pop_index AND f.ga_step = g.ga_step;'
        rs = self._select(sql, (), (int, int, float))
        
        return rs.fetchone()

    def get_biggest_tree(self):
        sql = 'SELECT g.ga_step, p.reg_step, g.pop_index, max(p.size), p.filled_nodes FROM grn g JOIN ptype_state p ON g.id = p.grn_id;'
        rs = self._select(sql, (), (int, int, int, int, int))

        return rs.fetchone()

    def get_highest_fill(self):
        sql = 'SELECT g.ga_step, p.reg_step, g.pop_index, p.size, max(p.filled_nodes) FROM grn g JOIN ptype_state p ON g.id = p.grn_id;'
        rs = self._select(sql, (), (int, int, int, int, int))

        return rs.fetchone()
