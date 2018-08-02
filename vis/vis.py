#!/usr/bin/python3

import sqlite3
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import shutil
import os
import sys
import argparse
from run import Run
from ipc import Ipc

IMG_DIR = '/home/wayne/Documents/school/thesis/unicellular-grn/data/images'
GENE_WIDTH = 0.2
GENE_HEIGHT = 0.25
FIG_WIDTH = 8
FIG_HEIGHT = 10
BAR_WIDTH = 0.8
#Kelly's 22 colours of max contrast
KELLY_COLOURS = ['#F2F3F4',
                 #'#222222', #thisis too close to black - text written over it doesn't show up well
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

def plot_avg_fitness(run_dir, conn):
    _plot_fitness(run_dir, conn, 'avg', 'Avg Fitness')

def plot_best_fitness(run_dir, conn):
    _plot_fitness(run_dir, conn, 'min', 'Best Fitness')

def _plot_fitness(run_dir, conn, sql_fcn, title):
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
    fig.savefig('{}/{}.png'.format(run_dir, title))

#note: column names have "ps." prefix (for protein_state)
def _get_conc_sql(run):
    sql = ''
    for i in range(run.num_genes):
        sql += 'ps.conc{}'.format(i)
        if i < run.num_genes - 1:
            sql += ', '

    return sql

def _plot_concs(ga_step, reg_step, pop_index, run, conn, ax):
    plt.sca(ax) #note: sca=set current axis

    conc_cols = _get_conc_sql(run)
    sql = 'SELECT p.pid, {} FROM grn JOIN protein_state ps ON grn.id = ps.grn_id JOIN protein p ON ps.protein_id = p.id WHERE grn.ga_step = ? AND ps.reg_step = ? AND grn.pop_index = ?;'.format(conc_cols)
    rs = conn.select(sql, (ga_step, reg_step, pop_index), [int] + [float] * run.num_genes)
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

    plt.xlabel('gene')
    plt.ylabel('concentration')

    return colours

def _draw_bindings(ga_step, reg_step, pop_index, run, conn, ax, colours):
    plt.sca(ax)
    
    sql = ('SELECT p.pid FROM grn JOIN gene g ON grn.id = g.grn_id JOIN ' +
           'gene_state gs ON gs.gene_id = g.id JOIN ' +
           'protein p ON gs.bound_protein = p.id ' + #note: gs.bound_protein is a *db* id (not a simulation id)
           'WHERE grn.ga_step = ? AND grn.pop_index = ? AND g.pos = ? AND gs.reg_step = ?;')

    for i in range(run.num_genes):
        rs = conn.select(sql, (ga_step, pop_index, i, reg_step), (int,))
        colour = 'white'
        text = ''
        if rs:
            pid = rs[0][0]
            text = "{}".format(pid)
            if pid in colours:
                colour_index = colours[pid]
                colour = KELLY_COLOURS[colour_index]
            else:
                colour = 'pink'
            
        rect = mpatches.Rectangle((i * GENE_WIDTH, -GENE_HEIGHT), GENE_WIDTH, GENE_HEIGHT, facecolor=colour, edgecolor='black')
        ax.add_patch(rect)
        if text:
            ax.text(i * GENE_WIDTH + GENE_WIDTH / 2.2, -GENE_HEIGHT + GENE_HEIGHT / 2.9, text)

def _draw_outputs(ga_step, reg_step, pop_index, run, conn, ax, colours):
    plt.sca(ax)
    
    sql = ('SELECT p.pid, g.output_rate FROM grn JOIN gene g ON grn.id = g.grn_id JOIN ' +
           'gene_state gs ON gs.gene_id = g.id JOIN ' +
           'protein p ON gs.active_output = p.id ' + #note: gs.active_output is a *db* id (not a simulation id)
           'WHERE grn.ga_step = ? AND grn.pop_index = ? AND g.pos = ? AND gs.reg_step = ?;')

    ys = []
    bar_colours = []
    bar_texts = []
    for i in range(run.num_genes):
        rs = conn.select(sql, (ga_step, pop_index, i, reg_step), (int, float))
        colour = 'white'
        text = ''
        if rs:
            pid, output_rate = rs[0]
            ys.append(output_rate)
            text = "{}".format(pid)
            if pid in colours:
                colour_index = colours[pid]
                colour = KELLY_COLOURS[colour_index]
            else:
                colour = 'pink'
                
            bar_colours.append(colour)
            bar_texts.append(text)
        else:
            ys.append(0)
            bar_texts.append('')
            bar_colours.append('white')
    
    xs = [i for i in range(run.num_genes)]
    ax.bar(xs, ys, BAR_WIDTH, color=bar_colours)
    plt.xlabel('gene')
    plt.ylabel('output rate')

    for i in range(run.num_genes):
        if bar_texts[i]:
            ax.text(xs[i], ys[i] / 2, bar_texts[i])

def draw_grn(run_dir, ga_step, reg_step, pop_index, run, conn):
    fig, ax = plt.subplots(2, 1, figsize=(FIG_WIDTH, FIG_HEIGHT))
    for i in range(run.num_genes):
        rect = mpatches.Rectangle((i * GENE_WIDTH, -GENE_HEIGHT), GENE_WIDTH, GENE_HEIGHT, fill=None)
        ax[0].add_patch(rect)

    ax[0].get_xaxis().set_visible(False)
    ax[0].spines['right'].set_visible(False)
    ax[0].spines['top'].set_visible(False)
    ax[0].spines['bottom'].set_visible(False)
    ax[0].set_ylim([-GENE_HEIGHT - 0.01, run.max_protein_conc])
    ax[0].set_xlim([0, run.num_genes * GENE_WIDTH + 0.01])

    colours = _plot_concs(ga_step, reg_step, pop_index, run, conn, ax[0])
    _draw_bindings(ga_step, reg_step, pop_index, run, conn, ax[0], colours)
    _draw_outputs(ga_step, reg_step, pop_index, run, conn, ax[1], colours)

    ax[0].legend(loc='center left', bbox_to_anchor=(1, 0.5))

    #plt.tight_layout()

    fig.savefig('{}/{}.png'.format(run_dir, reg_step))
    plt.close(fig)
    #plt.show()

def get_best(conn):
    sql = 'SELECT g.ga_step, g.pop_index, min(f.fitness) FROM grn g JOIN fitness f ON f.ga_step = g.ga_step AND f.pop_index = g.pop_index;'
    rs = conn.select(sql, (), (int, int, float))
    row = rs[0]

    return row

def main():
    parser = argparse.ArgumentParser(description="Graph the simulation results, using IPC to access the simulation's in-memory database.")
    parser.add_argument('--plot_reg_steps', type=str, default="false", help='set to "true" to generate graphs of regulatory steps')
    args = parser.parse_args()
    plot_reg_steps = args.plot_reg_steps.lower() == "true"
    
    conn = Ipc()

    run = Run(conn)
    run_name = "run{}".format(run.run_id)
    run_dir = "{}/{}".format(IMG_DIR, run_name)
    if os.path.exists(run_dir):
        shutil.rmtree(run_dir)
    os.makedirs(run_dir)
        
    plot_best_fitness(run_dir, conn)
    plot_avg_fitness(run_dir, conn)

    if plot_reg_steps:
        ga_step, pop_index, best_fitness = get_best(conn)
        for j in range(run.reg_steps):
            draw_grn(run_dir, ga_step, j, pop_index, run, conn)

    conn.close()

main()
