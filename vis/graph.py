#!/usr/bin/python3

import sqlite3
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import shutil
import os
import sys
import argparse
from graph.ipc import Ipc
from common.db import Db
from common.graph_gen import GraphGen
from common.run import Run

IMG_DIR = '/home/wayne/Documents/school/thesis/unicellular-grn/data/images'
DB_PATH = '/home/wayne/Documents/school/thesis/unicellular-grn/data/dbs/run0.db'

def main():
    parser = argparse.ArgumentParser(description="Graph the simulation results, using IPC to access the simulation's in-memory database.")
    parser.add_argument('--use_db', type=str, default='false', help='set to "true to use database instead of IPC')
    args = parser.parse_args()
    
    use_db = args.use_db.lower() == "true"

    if use_db:
        conn = Db(DB_PATH)
        run = Run(db=conn)
    else:
        conn = Ipc()
        run = Run(ipc=conn)
        
    run_name = "run{}".format(run.run_id)
    run_dir = "{}/{}".format(IMG_DIR, run_name)
    if os.path.exists(run_dir):
        shutil.rmtree(run_dir)
    os.makedirs(run_dir)

    if use_db:
        graph_gen = GraphGen(run, db=conn)
    else:
        graph_gen = GraphGen(run, ipc=conn, run_dir=run_dir)
        
    graph_gen.plot_cumulative_best_fitness()
    graph_gen.plot_avg_fitness()

    conn.close()

main()
