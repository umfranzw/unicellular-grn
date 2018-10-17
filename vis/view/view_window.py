import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
import PIL.Image

from step_button import StepButton
from db import Db
from tree_gen import TreeGen
from grn_gen import GrnGen
from run import Run

class ViewWindow(Gtk.Window):
    WIDTH = 800
    HEIGHT = 600
    
    def __init__(self):
        Gtk.Window.__init__(self, title='Viewer')
        self.set_default_size(ViewWindow.WIDTH, ViewWindow.HEIGHT)

        self.db = Db()
        self.tree_gen = TreeGen(self.db)
        self.grn_gen = GrnGen(self.db)
        self.run = Run(self.db)
        self.tree_cache = {}
        self.grn_cache = {}
        
        vbox = Gtk.VBox()
        paned = Gtk.Paned()
        left_scroll = Gtk.ScrolledWindow()
        self.tree_img = Gtk.Image()
        #self.tree_img.set_from_file('tree.png')
        left_scroll.add_with_viewport(self.tree_img)
        paned.add1(left_scroll)

        right_scroll = Gtk.ScrolledWindow()
        self.grn_img = Gtk.Image()
        #self.grn_img.set_from_file('grn.png')
        right_scroll.add_with_viewport(self.grn_img)
        paned.add2(right_scroll)
        paned.set_position(ViewWindow.WIDTH // 2)

        vbox.pack_start(paned, True, True, 0)
        vbox.pack_end(self._build_bottom_panel(), False, False, 0)
        
        self.add(vbox)

        self.update_views()

    def _build_bottom_panel(self):
        bbox = Gtk.ButtonBox(Gtk.ButtonBoxStyle.EDGE)
        self.save_tree_button = Gtk.Button(label='Save')
        self.save_tree_button.connect('clicked', self._save_img, self.tree_img)
        self.save_grn_button = Gtk.Button(label='Save')
        self.save_grn_button.connect('clicked', self._save_img, self.grn_img)
        
        bbox.add(self.save_tree_button)
        bbox.add(self.save_grn_button)
        
        grid = Gtk.Grid()
        
        ga_label = Gtk.Label('GA Step:')
        self.ga_spin = StepButton(self.db, 'grn', 'ga_step', entry_width=3)
        self.ga_spin.connect('change-value', self.update_views)

        reg_label = Gtk.Label('Reg Step:')
        self.reg_spin = StepButton(self.db, 'gene_state', 'reg_step', entry_width=3)
        self.reg_spin.connect('change-value', self.update_views)

        index_label = Gtk.Label('Pop Index:')
        self.index_spin = StepButton(self.db, 'fitness', 'pop_index', entry_width=3)
        self.index_spin.connect('change-value', self.update_views)

        grid.attach(ga_label, 0, 0, 1, 1)
        grid.attach(self.ga_spin, 1, 0, 1, 1)
        grid.attach(reg_label, 0, 1, 1, 1)
        grid.attach(self.reg_spin, 1, 1, 1, 1)
        grid.attach(index_label, 0, 2, 1, 1)
        grid.attach(self.index_spin, 1, 2, 1, 1)

        vbox = Gtk.VBox()
        sep = Gtk.Separator()
        vbox.add(bbox)
        vbox.add(sep)
        vbox.add(grid)
        
        return vbox

    def _get_img(self, cache, draw_fcn, ga_step, reg_step, pop_index):
        key = (ga_step, reg_step, pop_index)
        if key in cache:
            pixbuf = cache[key]
        else:
            pixbuf = draw_fcn()
            if pixbuf is not None:
                cache[key] = pixbuf

        return pixbuf

    def update_views(self, widget=None, val=None):
        ga_step = self.ga_spin.get_value()
        reg_step = self.reg_spin.get_value()
        pop_index = self.index_spin.get_value()

        tree_pixbuf = self._get_img(self.tree_cache,
                                    lambda: self.tree_gen.build_tree(ga_step, reg_step, pop_index),
                                    ga_step,
                                    reg_step,
                                    pop_index
        )
        
        if tree_pixbuf is None:
            self.tree_img.set_visible(False)
            self.save_tree_button.set_sensitive(False)
        else:
            self.tree_img.set_from_pixbuf(tree_pixbuf)
            self.tree_img.set_visible(True)
            self.save_tree_button.set_sensitive(True)

        grn_pixbuf = self._get_img(self.grn_cache,
                                   lambda: self.grn_gen.draw_grn(ga_step, reg_step, pop_index, self.run),
                                   ga_step,
                                   reg_step,
                                   pop_index
        )
        self.grn_img.set_from_pixbuf(grn_pixbuf)

    def _save_img(self, widget, img):
        pixbuf = img.get_pixbuf()
        height = pixbuf.get_height()
        width = pixbuf.get_width()
        data = pixbuf.get_pixels()
        pil_img = PIL.Image.frombytes('RGBA', (width, height), data)

        chooser = Gtk.FileChooserDialog(title='Save', action=Gtk.FileChooserAction.SAVE, buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_SAVE, Gtk.ResponseType.OK))
        chooser.set_do_overwrite_confirmation(True)
        response = chooser.run()

        filename = None
        if response == Gtk.ResponseType.OK:
            filename = chooser.get_file()
            pil_img.save(filename.get_path())
        chooser.destroy()
