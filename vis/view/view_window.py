import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GdkPixbuf, GLib
import PIL.Image
from collections import OrderedDict

from .step_button import StepButton
from common.db import Db
from .tree_gen import TreeGen
from common.graph_gen import GraphGen
from common.run import Run

class ViewWindow(Gtk.Window):
    WIDTH = 800
    HEIGHT = 600

    CACHE_SIZE = 100 #max number of images that can be stored in the cache
    
    def __init__(self, db_path):
        Gtk.Window.__init__(self, title='Viewer')
        self.set_default_size(ViewWindow.WIDTH, ViewWindow.HEIGHT)

        self.db = Db(db_path)
        self.run = Run(self.db)
        self.tree_gen = TreeGen(self.db)
        self.graph_gen = GraphGen(self.run, db=self.db)

        #we'll simulate an MRU (most recently used) cache using an OrderedDict
        #it's not the most efficient, but it's good enough for this application
        self.tree_cache = OrderedDict()
        self.grn_cache = OrderedDict()
        
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

    #cache images in png format to reduce size (pixbufs are big)
    #this means we have to convert pngbuf to pixbuf on cache fetch, but the conversion overhead is still significantly less than re-rendering the whole thing
    #this way we get a good balance of memory and time savings
    def _get_img(self, cache, draw_fcn, ga_step, reg_step, pop_index):
        key = (ga_step, reg_step, pop_index)
        pixbuf = None
        if key in cache:
            pixbuf = self._pngbuf_to_pixbuf(cache[key])
            cache.move_to_end(key) #record that this is now the most recently used
            
        else:
            pngbuf = draw_fcn()
            if pngbuf is not None:
                if len(cache) < ViewWindow.CACHE_SIZE:
                    cache[key] = pngbuf
                else:
                    #cache is full.
                    #Remove oldest element, which is at the beginning of the OrderedDict
                    start_key = cache.__iter__().__next__()
                    cache.pop(start_key)
                    #and add the new one at the end
                    cache[key] = pngbuf
                    
                pixbuf = self._pngbuf_to_pixbuf(pngbuf)

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
                                   lambda: self.graph_gen.draw_grn(ga_step, reg_step, pop_index, self.run),
                                   ga_step,
                                   reg_step,
                                   pop_index
        )
        self.grn_img.set_from_pixbuf(grn_pixbuf)

    def _pixbuf_to_pngbuf(self, pixbuf):
        pil_img = PIL.Image.frombytes('RGBA', (pixbuf.get_width(), pixbuf.get_height()), pixbuf.get_pixels())
        pngbuf = BytesIO()
        pil_img.save(pngbuf, format='png')
        pngbuf.seek(0)
        
        return pngbuf

    def _pngbuf_to_pixbuf(self, pngbuf):
        pil_img = PIL.Image.open(pngbuf)
        pixbuf = GdkPixbuf.Pixbuf.new_from_bytes(GLib.Bytes.new(pil_img.tobytes()), GdkPixbuf.Colorspace.RGB, True, 8, pil_img.width, pil_img.height, pil_img.width * 4)
        
        return pixbuf

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
