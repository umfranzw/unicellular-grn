import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

class ProteinFrame(Gtk.Frame):
    def __init__(self, db, title, filter_callback):
        Gtk.Frame.__init__(self)
        self.db = db
        
        title_label = Gtk.Label('<span weight="bold">{}</span>'.format(title))
        title_label.set_use_markup(True)
        self.set_label_widget(title_label)

        grid = Gtk.Grid()
        self.protein_scroll = Gtk.ScrolledWindow()
        self.protein_scroll.set_size_request(150, 20)
        self.checks_box = Gtk.VBox()
        self.protein_scroll.add_with_viewport(self.checks_box)
        grid.attach(self.protein_scroll, 0, 0, 1, 3)

        filter_button = Gtk.Button(label='Filter')
        filter_button.connect('clicked', filter_callback)
        grid.attach(filter_button, 1, 0, 1, 1)

        sel_button = Gtk.Button(label='Select All')
        sel_button.connect('clicked', lambda widget: self.set_checks(True))
        grid.attach(sel_button, 1, 1, 1, 1)
        
        clear_button = Gtk.Button(label='Clear All')
        clear_button.connect('clicked', lambda widget: self.set_checks(False))
        grid.attach(clear_button, 1, 2, 1, 1)
        
        self.add(grid)
        
        self.checked = {}
        self.cur_checked = {}

    def update_list(self, ga_step, reg_step, pop_index):
        for child in self.checks_box.get_children():
            self.checks_box.remove(child)
        
        sql = 'SELECT p.pid, p.seq FROM grn JOIN protein_state ps ON grn.id = ps.grn_id JOIN protein p ON ps.protein_id = p.id WHERE grn.ga_step = ? AND ps.reg_step = ? AND grn.pop_index = ?;'
        self.db.cur.execute(sql, (ga_step, reg_step, pop_index))

        self.cur_checked = {} #clear these
        for row in self.db.cur:
            pid, seq = row
            checkbutton = Gtk.CheckButton(label='{} ({})'.format(seq, pid))
            if pid not in self.checked:
                checkbutton.set_active(True)
                self.checked[pid] = True
                self.cur_checked[pid] = True

            else:
                checkbutton.set_active(self.checked[pid])
                self.cur_checked[pid] = self.checked[pid]
            
            checkbutton.connect('toggled', self._toggle_protein, pid)
            self.checks_box.add(checkbutton)

        self.checks_box.show_all()

    def _toggle_protein(self, checkbutton, pid):
        if checkbutton.get_active():
            self.checked[pid] = True
            self.cur_checked[pid] = True
        else:
            self.checked[pid] = False
            self.cur_checked[pid] = False

    def get_cur_checked(self):
        return [pid for pid in self.cur_checked]

    def get_cur_unchecked(self):
        return list(filter(lambda pid: not self.cur_checked[pid], self.cur_checked))

    def set_checks(self, active):
        for check in self.checks_box.get_children():
            check.set_active(active)
