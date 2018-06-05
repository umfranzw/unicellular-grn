#!/usr/bin/python3

import sqlite3
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from run import Run

DB_FILE = '../data/dbs/run0.db'
IMG_DIR = '../data/images'
GENE_WIDTH = 0.2
GENE_HEIGHT = 0.25
PLOT_WIDTH = 8
PLOT_HEIGHT = 8
KELLY_COLOURS = ['#F2F3F4', '#222222', '#F3C300', '#875692', '#F38400', '#A1CAF1', '#BE0032', '#C2B280', '#848482', '#008856', '#E68FAC', '#0067A5', '#F99379', '#604E97', '#F6A600', '#B3446C', '#DCD300', '#882D17', '#8DB600', '#654522', '#E25822', '#2B3D26'] #Kelly's 22 colours of max contrast
LINE_STYLES = ['-', '--', '-.', ':']

def plot_avg_fitness(conn):
    _plot_fitness(conn, 'avg', 'Avg Fitness')

def plot_best_fitness(conn):
    _plot_fitness(conn, 'min', 'Best Fitness')

def _plot_fitness(conn, sql_fcn, title):
    sql = 'SELECT ga_step, {}(fitness) FROM fitness GROUP BY ga_step;'.format(sql_fcn)
    rs = conn.execute(sql)
    xs = []
    ys = []
    for row in rs:
        ga_step, fitness = row
        xs.append(ga_step + 1)
        ys.append(fitness)

    plt.title(title)
    plt.xlabel('iteration')
    plt.ylabel('fitness')

    plt.plot(xs, ys)
    plt.show()

#note: column names have "ps." prefix (for protein_state)
def _get_conc_sql(run):
    sql = ''
    for i in range(run.num_genes):
        sql += 'ps.conc{}'.format(i)
        if i < run.num_genes - 1:
            sql += ', '

    return sql

def _plot_concs(ga_step, reg_step, pop_index, run, conn, ax):
    sql = 'SELECT p.pid, {} FROM grn JOIN protein_state ps ON grn.id = ps.grn_id JOIN protein p ON ps.protein_id = p.id WHERE grn.ga_step = ? AND ps.reg_step = ? AND grn.pop_index = ?;'.format(_get_conc_sql(run))
    rs = conn.execute(sql, (ga_step, reg_step, pop_index))
    colours = {} #maps pid to and index of KELLY_COLOURS

    xs = [0] + [(i + 0.5) * GENE_WIDTH for i in range(run.num_genes)] + [run.num_genes * GENE_WIDTH]
    for (i, row) in enumerate(rs):
        pid = row[0]
        concs = [0] + list(row[1:]) + [0]
        colour_index = i % len(KELLY_COLOURS)
        style_index = i // len(KELLY_COLOURS)
        ax.plot(xs, concs, color=KELLY_COLOURS[colour_index], linestyle=LINE_STYLES[style_index], label=pid)
        if style_index == 0:
            colours[pid] = colour_index

    return colours

def _draw_bindings(ga_step, reg_step, pop_index, run, conn, ax, colours):
    sql = ('SELECT p.pid FROM grn JOIN gene g ON grn.id = g.grn_id JOIN ' +
           'gene_state gs ON gs.gene_id = g.id JOIN ' +
           'protein p ON gs.bound_protein = p.id ' + #note: gs.bound_protein is a *db* id (not a simulation id)
           'WHERE grn.ga_step = ? AND grn.pop_index = ? AND g.pos = ? AND gs.reg_step = ?;')

    for i in range(run.num_genes):
        rs = conn.execute(sql, (ga_step, pop_index, i, reg_step))
        row = rs.fetchone()
        colour = 'white'
        text = ''
        if row:
            pid = row[0]
            text = "{}".format(pid)
            if pid in colours:
                colour_index = colours[pid]
                colour = KELLY_COLOURS[colour_index]
            else:
                #this isn't perfect, but it will do
                colour_index = (len(colours) + 1) % len(KELLY_COLOURS)
                colour = KELLY_COLOURS[colour_index]
            
        rect = mpatches.Rectangle((i * GENE_WIDTH, -GENE_HEIGHT), GENE_WIDTH, GENE_HEIGHT, facecolor=colour, edgecolor='black')
        ax.add_patch(rect)
        if text:
            plt.text(i * GENE_WIDTH + GENE_WIDTH / 2.2, -GENE_HEIGHT + GENE_HEIGHT / 2.9, text)

def _draw_outputs(ga_step, reg_step, pop_index, run, conn, ax, colours):
    xs = [1, 1, 1, 1, 1, 1, 1, 1]
    ax.bar(xs, run.max_protein_conc)

def draw_grn(ga_step, reg_step, pop_index, run, conn):
    fig, ax = plt.subplots(figsize=(PLOT_WIDTH, PLOT_HEIGHT))
    for i in range(run.num_genes):
        rect = mpatches.Rectangle((i * GENE_WIDTH, -GENE_HEIGHT), GENE_WIDTH, GENE_HEIGHT, fill=None)
        ax.add_patch(rect)

    ax.get_xaxis().set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)
    ax.spines['bottom'].set_visible(False)
    ax.set_ylim([-GENE_HEIGHT - 0.01, run.max_protein_conc])
    ax.set_xlim([0, run.num_genes * GENE_WIDTH + 0.01])

    colours = _plot_concs(ga_step, reg_step, pop_index, run, conn, ax)
    _draw_bindings(ga_step, reg_step, pop_index, run, conn, ax, colours)

    ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))

    #plt.tight_layout()

    fig.savefig('{}/{}.png'.format(IMG_DIR, reg_step))
    #plt.show()

def main():
    conn = sqlite3.connect(DB_FILE)
    run = Run(conn)
    #plot_best_fitness(conn)

    for i in range(run.reg_steps):
       draw_grn(0, i, 0, run, conn)

    conn.close()

main()
