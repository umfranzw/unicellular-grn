import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GdkPixbuf, GLib
import PIL.Image
from collections import OrderedDict
from view.protein_frame import ProteinFrame
import imageio

from .step_button import StepButton
from common.db import Db
from .tree_gen import TreeGen
from common.graph_gen import GraphGen
from common.run import Run
from .best_info import BestInfo

class ViewWindow(Gtk.Window):
    WIDTH = 800
    HEIGHT = 600

    CACHE_SIZE = 100 #max number of images that can be stored in the cache
    GIF_SCALE = 0.75
    
    def __init__(self, db_path):
        Gtk.Window.__init__(self, title='Viewer')
        self.set_default_size(ViewWindow.WIDTH, ViewWindow.HEIGHT)
        self.maximize()

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
        left_scroll.add_with_viewport(self.tree_img)
        paned.add1(left_scroll)

        right_scroll = Gtk.ScrolledWindow()
        self.grn_img = Gtk.Image()
        right_scroll.add_with_viewport(self.grn_img)
        paned.add2(right_scroll)
        paned.set_position(ViewWindow.WIDTH // 2)

        vbox.pack_start(paned, True, True, 0)
        vbox.pack_end(self._build_bottom_panel(), False, False, 0)
        
        self.add(vbox)

        self.show_all()

        self.update_views()

    def _build_bottom_panel(self):
        bbox = Gtk.ButtonBox(Gtk.ButtonBoxStyle.EDGE)
        self.save_tree_button = Gtk.Button(label='Save')
        self.save_tree_button.connect('clicked', self._save_img, self.tree_img)
        # self.save_both_button = Gtk.Button(label='Save Both')
        # self.save_both_button.connect('clicked', self._save_both, self.tree_img, self.grn_img)
        # self.save_gif_button = Gtk.Button(label='Save GIF')
        # self.save_gif_button.connect('clicked', self._save_gif)
        self.save_grn_button = Gtk.Button(label='Save')
        self.save_grn_button.connect('clicked', self._save_img, self.grn_img)
        
        bbox.add(self.save_tree_button)
        #bbox.add(self.save_gif_button)
        bbox.add(self.save_grn_button)

        #button grid
        button_grid = Gtk.Grid()
        
        ga_label = Gtk.Label('GA Step:')
        self.ga_spin = StepButton(self.db, 'grn', 'ga_step', entry_width=3)
        self.ga_spin.connect('change-value', self.update_views)

        reg_label = Gtk.Label('Reg Step:')
        self.reg_spin = StepButton(self.db, 'gene_state', 'reg_step', entry_width=3)
        self.reg_spin.connect('change-value', self.update_views)

        index_label = Gtk.Label('Pop Index:')
        self.index_spin = StepButton(self.db, 'fitness', 'pop_index', entry_width=3)
        self.index_spin.connect('change-value', self.update_views)

        precomp_button = Gtk.Button(label='Precompute')
        precomp_button.connect('clicked', self._precomp_reg_steps)

        button_grid.attach(ga_label, 0, 0, 1, 1)
        button_grid.attach(self.ga_spin, 1, 0, 1, 1)
        button_grid.attach(reg_label, 0, 1, 1, 1)
        button_grid.attach(self.reg_spin, 1, 1, 1, 1)
        button_grid.attach(index_label, 0, 2, 1, 1)
        button_grid.attach(self.index_spin, 1, 2, 1, 1)
        button_grid.attach(precomp_button, 2, 1, 1, 1)

        #fittest grid
        info = BestInfo(db=self.db)

        grid_title = 'Fittest GRN'
        titles = ['ga_step:', 'pop_index:', 'fitness:']
        vals = info.get_fittest_grn()
        fittest_frame = self._build_info_frame(grid_title, titles, vals)
        
        #biggest grid
        grid_title = 'Biggest Tree'
        titles = ['ga_step:', 'reg_step:', 'pop_index:', 'size:', 'filled_nodes:']
        vals = info.get_biggest_tree()
        biggest_frame = self._build_info_frame(grid_title, titles, vals)

        #filled grid
        grid_title = 'Highest Fill'
        titles = ['ga_step:', 'reg_step:', 'pop_index:', 'size:', 'filled_nodes:']
        vals = info.get_highest_fill()
        filled_frame = self._build_info_frame(grid_title, titles, vals)

        self.protein_frame = ProteinFrame(self.db, 'Proteins', self._update_grn_view)

        #arrange grids
        vbox = Gtk.VBox()
        sep = Gtk.Separator()
        vbox.add(sep)
        vbox.add(bbox)

        hbox = Gtk.HBox()
        hbox.pack_start(button_grid, False, False, 0)
        hbox.pack_end(fittest_frame, False, False, 1)
        hbox.pack_end(biggest_frame, False, False, 5)
        hbox.pack_end(filled_frame, False, False, 5)
        hbox.pack_end(self.protein_frame, False, False, 5)

        vbox.add(hbox)

        return vbox

    def _precomp_reg_steps(self, widget):
        ga_step = self.ga_spin.get_value()
        pop_index = self.index_spin.get_value()
        
        for i in range(self.run.reg_steps):
            self._get_img(self.tree_cache,
                          lambda: self.tree_gen.build_tree(ga_step, i, pop_index),
                          ga_step,
                          i,
                          pop_index
            )

            self._get_img(self.grn_cache,
                          lambda: self.graph_gen.draw_grn(ga_step, i, pop_index, self.run),
                          ga_step,
                          i,
                          pop_index
            )

    def _build_info_frame(self, frame_title, titles, vals):
        grid = Gtk.Grid()
        grid.set_column_spacing(5)
        
        for i in range(len(titles)):
            title = Gtk.Label(titles[i])
            title.set_halign(Gtk.Align.END)
            val = Gtk.Label(str(vals[i]))
            val.set_halign(Gtk.Align.START)
            grid.attach(title, 0, i, 1, 1)
            grid.attach(val, 1, i, 1, 1)

        frame = Gtk.Frame()
        frame_label = Gtk.Label('<span weight="bold">{}</span>'.format(frame_title))
        frame_label.set_use_markup(True)
        frame.set_label_widget(frame_label)
        frame.add(grid)

        return frame

    #cache images in png format to reduce size (pixbufs are big)
    #this means we have to convert pngbuf to pixbuf on cache fetch, but the conversion overhead is still significantly less than re-rendering the whole thing
    #this way we get a good balance of memory and time savings
    def _get_img(self, cache, draw_fcn, ga_step, reg_step, pop_index, bypass_cache=False):
        key = (ga_step, reg_step, pop_index)
        pixbuf = None

        if bypass_cache:
            pngbuf = draw_fcn()
            pixbuf = self._pngbuf_to_pixbuf(pngbuf)

        else:
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
        self.protein_frame.update_list(ga_step, reg_step, pop_index)

        self._update_tree_view()
        self._update_grn_view()

    def _update_grn_view(self, widget=None):
        ga_step = self.ga_spin.get_value()
        reg_step = self.reg_spin.get_value()
        pop_index = self.index_spin.get_value()
        unchecked = self.protein_frame.get_cur_unchecked()

        bypass_cache = len(unchecked) > 0
        pixbuf = self._get_img(self.grn_cache,
                               lambda: self.graph_gen.draw_grn(ga_step, reg_step, pop_index, self.run, unchecked),
                               ga_step,
                               reg_step,
                               pop_index,
                               bypass_cache=bypass_cache
        )
        
        self.grn_img.set_from_pixbuf(pixbuf)

    def _update_tree_view(self, widget=None):
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

    def _save_both(self, widget, img1, img2):
        pil_imgs = []
        big_width = 0
        big_height = 0
        for img in (img1, img2):
            pixbuf = img.get_pixbuf()
            height = pixbuf.get_height()
            width = pixbuf.get_width()
            data = pixbuf.get_pixels()
            pil_img = PIL.Image.frombytes('RGBA', (width, height), data)
            big_width += pil_img.width
            big_height = max(big_height, pil_img.height)
            pil_imgs.append(pil_img)

        big_img = PIL.Image.new('RGBA', (big_width, big_height))

        x_offset = 0
        for im in pil_imgs:
            big_img.paste(im, (x_offset, 0))
            x_offset += im.width

        chooser = Gtk.FileChooserDialog(title='Save', action=Gtk.FileChooserAction.SAVE, buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_SAVE, Gtk.ResponseType.OK))
        chooser.set_do_overwrite_confirmation(True)
        response = chooser.run()

        filename = None
        if response == Gtk.ResponseType.OK:
            filename = chooser.get_file()
            big_img.save(filename.get_path())
        chooser.destroy()

    def _save_gif(self, widget):
        ga_step = self.ga_spin.get_value()
        pop_index = self.index_spin.get_value()

        trees = []
        grns = []
        tree_size = [0, 0]
        grn_size = [0, 0]
        #generate images
        print('Generating...')
        for reg_step in range(-1, self.run.reg_steps):
            print(reg_step)
            grn_pngbuf = self.graph_gen.draw_grn(ga_step, reg_step, pop_index, self.run)
            grn_img = PIL.Image.open(grn_pngbuf)
            grn_size[0] = max(grn_size[0], grn_img.width)
            grn_size[1] = max(grn_size[1], grn_img.height)
            grns.append(grn_img)

            tree_pngbuf = self.tree_gen.build_tree(ga_step, reg_step, pop_index)
            if tree_pngbuf is not None:
                tree_img = PIL.Image.open(tree_pngbuf)
                tree_size[0] = max(tree_size[0], tree_img.width)
                tree_size[1] = max(tree_size[1], tree_img.height)
                trees.append(tree_img)
            else:
                trees.append(None)

        #combine images
        frames = []
        big_size = (tree_size[0] + grn_size[0], max(tree_size[1], grn_size[1]))
        print('Combining...')
        for i in range(len(trees)):
            print(i - 1)
            big_img = PIL.Image.new('RGBA', big_size, color=(255, 255, 255, 0))
            if trees[i] is not None:
                big_img.paste(trees[i], (0, 0))

            big_img.paste(grns[i], (tree_size[0], 0))
            if ViewWindow.GIF_SCALE != 1:
                big_img = big_img.resize((int(big_img.width * ViewWindow.GIF_SCALE), int(big_img.height * ViewWindow.GIF_SCALE)))
            big_img.save('/tmp/.gif.png', format='png')
            frames.append(imageio.imread('/tmp/.gif.png'))

        print('Saving...')
        imageio.mimsave('movie.gif', frames, duration=0.25)
        print('Done')
