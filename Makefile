# tsim - Tiny Simulator v0.1
# Copyright (C) 1998 by Mark-André Hopf
#-----------------------------------------------------------------------------

PRGFILE		= tsim

SRCS        = MainWindow.cc ToolBar.cc Paper.cc DNDShape.cc \
			  Simulate.cc Shape.cc Act.cc \
			  textarea.cc membuf.cc

LIBS		=

CLEAN       =

DEFINES		=

include ../toad/conf/MakefilePrg

# make sure we link again when TOAD has been modified
#-----------------------------------------------------
$(PRGFILE): ../toad/lib/libtoad.a

# need this to avoid some troubles with libg++-2.7.0
#-----------------------------------------------------
membuf.o: membuf.cc membuf.hh
	@echo compiling membuf.cc without RTTI ...
	@$(CC) -o membuf.o -c membuf.cc
