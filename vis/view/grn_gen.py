import gi
from gi.repository import GdkPixbuf, GLib
import PIL.Image
from io import BytesIO
import sqlite3
import matplotlib
matplotlib.use('svg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

class GrnGen():
    GENE_WIDTH = 0.2
    GENE_HEIGHT = 0.25
    FIG_WIDTH = 8
    FIG_HEIGHT = 10
    BAR_WIDTH = 0.8
    
    #Kelly's 22 colours of max contrast
    KELLY_COLOURS = ['#F2F3F4',
                     #'#222222', #this is too close to black - text written over it doesn't show up well
                     '#F3C300',
                     '#875692',
                     '#F38400',
                     '#A1CAF1',
                     '#BE0032',
                     '#C2B280',
                     '#848482',
                     '#008856',
                     '#E68FAC',
                     '#0067A5',
                     '#F99379',
                     '#604E97',
                     '#F6A600',
                     '#B3446C',
                     '#DCD300',
                     '#882D17',
                     '#8DB600',
                     '#654522',
                     '#E25822',
                     '#2B3D26']
    LINE_STYLES = ['-', '--', '-.', ':']

    def __init__(self, db):
        self.db = db

    def _fig_to_pixbuf(self, fig):
        buf = BytesIO()
        fig.savefig(buf, format='png')
        buf.seek(0)
        img = PIL.Image.open(buf)
        #img = img.resize((400, 300))
        width, height = img.size
        glib_data = GLib.Bytes.new(img.tobytes())
        pixbuf = GdkPixbuf.Pixbuf.new_from_bytes(glib_data, GdkPixbuf.Colorspace.RGB, True, 8, width, height, width * 4)
        buf.close()

        return pixbuf

    #note: column names have "ps." prefix (for protein_state)
    def _get_conc_sql(self, run):
        sql = ''
        for i in range(run.num_genes):
            sql += 'ps.conc{}'.format(i)
            if i < run.num_genes - 1:
                sql += ', '

        return sql

    def _plot_concs(self, ga_step, reg_step, pop_index, run, ax):
        plt.sca(ax) #note: sca=set current axis

        conc_cols = self._get_conc_sql(run)
        sql = 'SELECT p.pid, {} FROM grn JOIN protein_state ps ON grn.id = ps.grn_id JOIN protein p ON ps.protein_id = p.id WHERE grn.ga_step = ? AND ps.reg_step = ? AND grn.pop_index = ?;'.format(conc_cols)
        self.db.cur.execute(sql, (ga_step, reg_step, pop_index))
        colours = {} #maps pid to and index of GrnGen.KELLY_COLOURS

        xs = [0] + [(i + 0.5) * GrnGen.GENE_WIDTH for i in range(run.num_genes)] + [run.num_genes * GrnGen.GENE_WIDTH]
        for (i, row) in enumerate(self.db.cur):
            pid = row[0]
            concs = [0] + list(row[1:]) + [0]
            colour_index = i % len(GrnGen.KELLY_COLOURS)
            style_index = i // len(GrnGen.KELLY_COLOURS)
            ax.plot(xs, concs, color=GrnGen.KELLY_COLOURS[colour_index], linestyle=GrnGen.LINE_STYLES[style_index], label=pid)
            if style_index == 0:
                colours[pid] = colour_index

        plt.xlabel('gene')
        plt.ylabel('concentration')

        return colours

    def _draw_bindings(self, ga_step, reg_step, pop_index, run, ax, colours):
        plt.sca(ax)

        sql = ('SELECT p.pid FROM grn JOIN gene g ON grn.id = g.grn_id JOIN ' +
               'gene_state gs ON gs.gene_id = g.id JOIN ' +
               'protein p ON gs.bound_protein = p.id ' + #note: gs.bound_protein is a *db* id (not a simulation id)
               'WHERE grn.ga_step = ? AND grn.pop_index = ? AND g.pos = ? AND gs.reg_step = ?;')

        for i in range(run.num_genes):
            self.db.cur.execute(sql, (ga_step, pop_index, i, reg_step))
            colour = 'white'
            text = ''
            row = self.db.cur.fetchone()
            if row:
                pid = row[0]
                text = "{}".format(pid)
                if pid in colours:
                    colour_index = colours[pid]
                    colour = GrnGen.KELLY_COLOURS[colour_index]
                else:
                    colour = 'pink'

            rect = mpatches.Rectangle((i * GrnGen.GENE_WIDTH, -GrnGen.GENE_HEIGHT), GrnGen.GENE_WIDTH, GrnGen.GENE_HEIGHT, facecolor=colour, edgecolor='black')
            ax.add_patch(rect)
            if text:
                ax.text(i * GrnGen.GENE_WIDTH + GrnGen.GENE_WIDTH / 2.2, -GrnGen.GENE_HEIGHT + GrnGen.GENE_HEIGHT / 2.9, text)

    def _draw_outputs(self, ga_step, reg_step, pop_index, run, ax, colours):
        plt.sca(ax)

        sql = ('SELECT p.pid, g.output_rate FROM grn JOIN gene g ON grn.id = g.grn_id JOIN ' +
               'gene_state gs ON gs.gene_id = g.id JOIN ' +
               'protein p ON gs.active_output = p.id ' + #note: gs.active_output is a *db* id (not a simulation id)
               'WHERE grn.ga_step = ? AND grn.pop_index = ? AND g.pos = ? AND gs.reg_step = ?;')

        ys = []
        bar_colours = []
        bar_texts = []
        for i in range(run.num_genes):
            self.db.cur.execute(sql, (ga_step, pop_index, i, reg_step))
            colour = 'white'
            text = ''
            row = self.db.cur.fetchone()
            if row:
                pid, output_rate = row
                ys.append(output_rate)
                text = "{}".format(pid)
                if pid in colours:
                    colour_index = colours[pid]
                    colour = GrnGen.KELLY_COLOURS[colour_index]
                else:
                    colour = 'pink'

                bar_colours.append(colour)
                bar_texts.append(text)
            else:
                ys.append(0)
                bar_texts.append('')
                bar_colours.append('white')

        xs = [i for i in range(run.num_genes)]
        ax.bar(xs, ys, GrnGen.BAR_WIDTH, color=bar_colours)
        plt.xlabel('gene')
        plt.ylabel('output rate')

        for i in range(run.num_genes):
            if bar_texts[i]:
                ax.text(xs[i], ys[i] / 2, bar_texts[i])

    def draw_grn(self, ga_step, reg_step, pop_index, run):
        fig, ax = plt.subplots(2, 1, figsize=(GrnGen.FIG_WIDTH, GrnGen.FIG_HEIGHT))
        for i in range(run.num_genes):
            rect = mpatches.Rectangle((i * GrnGen.GENE_WIDTH, -GrnGen.GENE_HEIGHT), GrnGen.GENE_WIDTH, GrnGen.GENE_HEIGHT, fill=None)
            ax[0].add_patch(rect)

        ax[0].get_xaxis().set_visible(False)
        ax[0].spines['right'].set_visible(False)
        ax[0].spines['top'].set_visible(False)
        ax[0].spines['bottom'].set_visible(False)
        ax[0].set_ylim([-GrnGen.GENE_HEIGHT - 0.01, run.max_protein_conc])
        ax[0].set_xlim([0, run.num_genes * GrnGen.GENE_WIDTH + 0.01])

        colours = self._plot_concs(ga_step, reg_step, pop_index, run,  ax[0])
        self._draw_bindings(ga_step, reg_step, pop_index, run, ax[0], colours)
        self._draw_outputs(ga_step, reg_step, pop_index, run, ax[1], colours)

        ax[0].legend(loc='center left', bbox_to_anchor=(1, 0.5))

        #plt.tight_layout()

        pixbuf = self._fig_to_pixbuf(fig)
        plt.close(fig)
        
        return pixbuf
