import gi
from gi.repository import GdkPixbuf, GLib
from graphviz import Digraph
import PIL.Image
from io import BytesIO

class TreeGen():
    def __init__(self, db):
        self.db = db

    def build_tree(self, ga_step, reg_step, pop_index):
        pixbuf = None
        
        graph = Digraph(format='png', edge_attr={'dir': 'none'})
        sql = 'SELECT t.id FROM grn g JOIN ptype_state p ON g.id = p.grn_id JOIN tree t on p.id = t.ptype_state_id WHERE g.ga_step = ? AND g.pop_index = ? AND p.reg_step = ?;'
        self.db.cur.execute(sql, (ga_step, pop_index, reg_step))
        row = self.db.cur.fetchone()
        if row:
            #find root
            tree_id = row[0]
            sql = 'SELECT id, descr FROM node WHERE tree_id = ? AND parent_id IS NULL;'
            self.db.cur.execute(sql, (tree_id,))
            row = self.db.cur.fetchone()
            if row:
                node_id, desc = row
                if desc is None:
                    desc = ''
                self._create_node(graph, tree_id, node_id, desc)

                #render in memory
                dot_data = BytesIO(graph.pipe())
                img = PIL.Image.open(dot_data)
                #img = img.resize((100, 100))
                width, height = img.size
                glib_data = GLib.Bytes.new(img.tobytes())
                pixbuf = GdkPixbuf.Pixbuf.new_from_bytes(glib_data, GdkPixbuf.Colorspace.RGB, True, 8, width, height, width * 4)

        return pixbuf

    def _create_node(self, graph, tree_id, node_id, desc):
        graph.node(str(node_id), desc)

        #find any children and create nodes for them
        sql = 'SELECT id, descr FROM node WHERE tree_id = ? AND parent_id = ?;'
        self.db.cur.execute(sql, (tree_id, node_id))
        rows = list(self.db.cur)
        for row in rows:
            child_id, child_desc = row
            if child_desc is None:
                child_desc = ''
            self._create_node(graph, tree_id, child_id, child_desc)
            graph.edge(str(node_id), str(child_id))
