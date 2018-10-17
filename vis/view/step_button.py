import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GObject

import math

class StepButton(Gtk.Grid):
    def __init__(self, db, table, col, entry_width=None, *args, **kwargs):
        super().__init__(*args, *kwargs)
        
        sql = 'SELECT distinct {} FROM {} ORDER BY {};'.format(col, table, col)
        db.cur.execute(sql)
        self.steps = []
        for row in db.cur:
            self.steps.append(row[0])

        self.min_step = -1
        self.max_step = -1
        if self.steps:
           self.min_step = self.steps[0]
           self.max_step = self.steps[-1]
           
        self.index = 0
        self.entry = self._build(entry_width)

    def _build(self, entry_width):
        if entry_width is None:
            entry_width = 2
            if self.max_step > 1:
                entry_width = math.ceil(math.log10(self.max_step))
                
        entry = Gtk.Entry()
        entry.set_width_chars(entry_width)
        entry.set_text(str(self.min_step))
        entry.connect('activate', self._set_text)
        entry.connect('focus-out-event', lambda entry, event: self._set_text(entry))
        
        down = Gtk.Button(label='-')
        down.connect('clicked', self._spin, -1)
        up = Gtk.Button(label='+')
        up.connect('clicked', self._spin, 1)
        
        self.attach(entry, 0, 0, 1, 1)
        self.attach(down, 1, 0, 1, 1)
        self.attach(up, 2, 0, 1, 1)

        return entry

    def _spin(self, widget, delta):
        new_index = self.index + delta
        if new_index < len(self.steps) and new_index >= 0:
            self.index = new_index
            self.entry.set_text(str(self.steps[self.index]))
            self.emit('change-value', self.steps[self.index])

    def _set_text(self, entry):
        raw_val = entry.get_text()
        
        try:
            #note: both lines below throw ValueError exceptions if something goes awry
            val = int(raw_val)
            new_index = self.steps.index(val)
            if new_index != self.index:
                self.index = new_index
                self.emit('change-value', self.steps[self.index])
            
        except ValueError:
            val = self.steps[self.index] if self.steps else self.min_step

        entry.set_text(str(val))

    def get_value(self):
        return self.steps[self.index] if self.steps else self.min_step

#Many Bothans died to bring us this information.
GObject.signal_new('change-value', StepButton, GObject.SIGNAL_RUN_LAST, GObject.TYPE_NONE, (GObject.TYPE_INT,))
