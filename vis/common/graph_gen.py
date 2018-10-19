import gi
from gi.repository import GdkPixbuf, GLib
import PIL.Image
from io import BytesIO
import sqlite3
import matplotlib
matplotlib.use('svg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

class GraphGen():
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

    def __init__(self, run, db=None, ipc=None, run_dir=None):
        self.run = run
        
        if db is not None:
            self.db = db
            self.ipc = None
            self.run_dir = None
            
        elif ipc is not None:
            self.db = None
            self.ipc = ipc
            self.run_dir = run_dir
            
        else:
            print('No database connection provided.')

    def plot_avg_fitness():
        return self._plot_fitness('avg', 'Avg Fitness')

    def plot_best_fitness():
        return self._plot_fitness('min', 'Best Fitness')

    def plot_cumulative_best_fitness():
        sql = ('SELECT f1.ga_step, (' +
               'SELECT min(f2.fitness) ' +
               'FROM fitness f2 ' +
               'WHERE f2.ga_step <= f1.ga_step' +
               ') AS min_fitness FROM fitness f1 ' +
               'GROUP BY f1.ga_step;')

        rs = self._select(sql, (), (int, float))
        xs = []
        ys = []
        for row in rs:
            ga_step, fitness = row
            xs.append(ga_step + 1)
            ys.append(fitness)

        plt.title('Best Fitness')
        plt.xlabel('iteration')
        plt.ylabel('fitness')

        fig, ax = plt.subplots()
        ax.plot(xs, ys)
        path = '{}/{}.png'.format(self.run_dir, 'Best Fitness')

        return self._process_fig(fig, path)

    def _plot_fitness(sql_fcn, title):
        sql = 'SELECT ga_step, {}(fitness) FROM fitness GROUP BY ga_step;'.format(sql_fcn)
        rs = conn.select(sql, (), (int, float))
        xs = []
        ys = []
        for row in rs:
            ga_step, fitness = row
            xs.append(ga_step + 1)
            ys.append(fitness)

        plt.title(title)
        plt.xlabel('iteration')
        plt.ylabel('fitness')

        fig, ax = plt.subplots()
        ax.plot(xs, ys)
        path = '{}/{}.png'.format(self.run_dir, title)

        return self._process_fig(fig, path)

    def _fig_to_pngbuf(self, fig):
        buf = BytesIO()
        fig.savefig(buf, format='png')
        buf.seek(0)
        # img = PIL.Image.open(buf)
        # #img = img.resize((400, 300))
        # width, height = img.size
        # glib_data = GLib.Bytes.new(img.tobytes())
        # pixbuf = GdkPixbuf.Pixbuf.new_from_bytes(glib_data, GdkPixbuf.Colorspace.RGB, True, 8, width, height, width * 4)
        # buf.close()

        return buf

    def _process_fig(self, path):
        result = None
        if self.run_dir:
            fig.savefig(path)

        else:
            result = self._fig_to_pngbuf(fig)

        return result
    
    def get_best():
        sql = 'SELECT g.ga_step, g.pop_index, min(f.fitness) FROM grn g JOIN fitness f ON f.ga_step = g.ga_step AND f.pop_index = g.pop_index;'
        rs = self._select(sql, (), (int, int, float))

        return rs[0]

    #note: column names have "ps." prefix (for protein_state)
    def _get_conc_sql(self, run):
        sql = ''
        for i in range(self.run.num_genes):
            sql += 'ps.conc{}'.format(i)
            if i < self.run.num_genes - 1:
                sql += ', '

        return sql

    def _select(self, sql, args, result_types=None):
        rs = None
        if self.db:
            self.db.cur.execute(sql, args)
            rs = self.db.cur

        else:
            rs = self.ipc.select(sql, args, result_types)

        return rs

    def _plot_concs(self, ga_step, reg_step, pop_index, run, ax):
        plt.sca(ax) #note: sca=set current axis

        conc_cols = self._get_conc_sql(run)
        sql = 'SELECT p.pid, {} FROM grn JOIN protein_state ps ON grn.id = ps.grn_id JOIN protein p ON ps.protein_id = p.id WHERE grn.ga_step = ? AND ps.reg_step = ? AND grn.pop_index = ?;'.format(conc_cols)
        types = [int] + [float] * self.run.num_genes
        self._select(sql, (ga_step, reg_step, pop_index), types)
        colours = {} #maps pid to and index of GraphGen.KELLY_COLOURS

        xs = [0] + [(i + 0.5) * GraphGen.GENE_WIDTH for i in range(self.run.num_genes)] + [self.run.num_genes * GraphGen.GENE_WIDTH]
        for (i, row) in enumerate(self.db.cur):
            pid = row[0]
            concs = [0] + list(row[1:]) + [0]
            colour_index = i % len(GraphGen.KELLY_COLOURS)
            style_index = i // len(GraphGen.KELLY_COLOURS)
            ax.plot(xs, concs, color=GraphGen.KELLY_COLOURS[colour_index], linestyle=GraphGen.LINE_STYLES[style_index], label=pid)
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

        for i in range(self.run.num_genes):
            self._select(sql, (ga_step, pop_index, i, reg_step), (int,))
            colour = 'white'
            text = ''
            row = self.db.cur.fetchone()
            if row:
                pid = row[0]
                text = "{}".format(pid)
                if pid in colours:
                    colour_index = colours[pid]
                    colour = GraphGen.KELLY_COLOURS[colour_index]
                else:
                    colour = 'pink'

            rect = mpatches.Rectangle((i * GraphGen.GENE_WIDTH, -GraphGen.GENE_HEIGHT), GraphGen.GENE_WIDTH, GraphGen.GENE_HEIGHT, facecolor=colour, edgecolor='black')
            ax.add_patch(rect)
            if text:
                ax.text(i * GraphGen.GENE_WIDTH + GraphGen.GENE_WIDTH / 2.2, -GraphGen.GENE_HEIGHT + GraphGen.GENE_HEIGHT / 2.9, text)

    def _draw_outputs(self, ga_step, reg_step, pop_index, run, ax, colours):
        plt.sca(ax)

        sql = ('SELECT p.pid, g.output_rate FROM grn JOIN gene g ON grn.id = g.grn_id JOIN ' +
               'gene_state gs ON gs.gene_id = g.id JOIN ' +
               'protein p ON gs.active_output = p.id ' + #note: gs.active_output is a *db* id (not a simulation id)
               'WHERE grn.ga_step = ? AND grn.pop_index = ? AND g.pos = ? AND gs.reg_step = ?;')

        ys = []
        bar_colours = []
        bar_texts = []
        for i in range(self.run.num_genes):
            self._select(sql, (ga_step, pop_index, i, reg_step), (int, float))
            colour = 'white'
            text = ''
            row = self.db.cur.fetchone()
            if row:
                pid, output_rate = row
                ys.append(output_rate)
                text = "{}".format(pid)
                if pid in colours:
                    colour_index = colours[pid]
                    colour = GraphGen.KELLY_COLOURS[colour_index]
                else:
                    colour = 'pink'

                bar_colours.append(colour)
                bar_texts.append(text)
            else:
                ys.append(0)
                bar_texts.append('')
                bar_colours.append('white')

        xs = [i for i in range(self.run.num_genes)]
        ax.bar(xs, ys, GraphGen.BAR_WIDTH, color=bar_colours)
        plt.xlabel('gene')
        plt.ylabel('output rate')

        for i in range(self.run.num_genes):
            if bar_texts[i]:
                ax.text(xs[i], ys[i] / 2, bar_texts[i])

    def draw_grn(self, ga_step, reg_step, pop_index, run):
        fig, ax = plt.subplots(2, 1, figsize=(GraphGen.FIG_WIDTH, GraphGen.FIG_HEIGHT))
        for i in range(self.run.num_genes):
            rect = mpatches.Rectangle((i * GraphGen.GENE_WIDTH, -GraphGen.GENE_HEIGHT), GraphGen.GENE_WIDTH, GraphGen.GENE_HEIGHT, fill=None)
            ax[0].add_patch(rect)

        ax[0].get_xaxis().set_visible(False)
        ax[0].spines['right'].set_visible(False)
        ax[0].spines['top'].set_visible(False)
        ax[0].spines['bottom'].set_visible(False)
        ax[0].set_ylim([-GraphGen.GENE_HEIGHT - 0.01, self.run.max_protein_conc])
        ax[0].set_xlim([0, self.run.num_genes * GraphGen.GENE_WIDTH + 0.01])

        colours = self._plot_concs(ga_step, reg_step, pop_index, run,  ax[0])
        self._draw_bindings(ga_step, reg_step, pop_index, run, ax[0], colours)
        self._draw_outputs(ga_step, reg_step, pop_index, run, ax[1], colours)

        ax[0].legend(loc='center left', bbox_to_anchor=(1, 0.5))

        #plt.tight_layout()

        pixbuf = self._fig_to_pngbuf(fig)
        plt.close(fig)
        
        return pixbuf
