#!/usr/bin/python

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

from view.view_window import ViewWindow

DB_PATH = '/home/wayne/Documents/school/thesis/unicellular-grn/data/dbs/run0.db'

def main():
    win = ViewWindow(DB_PATH)
    win.connect("destroy", Gtk.main_quit)
    Gtk.main()

main()
