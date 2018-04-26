#------------------------------------------------------------------------
#
# File  : Makefile
#
# This is the Makefile for contributed code of the E distribution. This is
# more convoluted than I like because it deals with PicoSat, which follows
# some conventions that are different frome the ones in E.
#
# Author: Stephan Schulz
#
# Created: Mon Nov 27 12:28:52 CET 2017
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PICOSAT = picosat-965

all: picosat liblinear libsvd

# Remove all automatically generated files

clean: $(PICOSAT)/makefile
	cd $(PICOSAT); $(MAKE) clean
	make -C liblinear clean
	make -C svdlibc clean

# Services (provided by the master Makefile)

include ../Makefile.services

config:
	cd $(PICOSAT); ./configure.sh

$(PICOSAT)/makefile:
	make config

picosat: $(PICOSAT)/makefile
	cd $(PICOSAT); $(MAKE); $(MAKE)

liblinear: liblinear/liblinear.a

liblinear/liblinear.a:
	make -C liblinear lib

libsvd: svdlibc/libsdv.a

svdlibc/libsdv.a:
	make -C svdlibc

