#!/usr/bin/python

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

from view_window import *

def main():
    win = ViewWindow()
    win.connect("destroy", Gtk.main_quit)
    win.show_all()
    Gtk.main()

main()
