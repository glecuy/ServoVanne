#*****************************************************************************
#
# File : Makefile
# ----
#
# Copyright Statement :
# ------------------
#
# Project :
# -------
#
# Description :
# -----------
# This file defines rules to
#
#
# Version : %Z% %M% %G% %I%
# -------
#
# Modification History :
# --------------------
#
# Date      Major Version   Author          Comment
# ----      -------------   ------          -------
# 28/09/99  1.0             GL                Initial Version
#  XP Compatible !!
#
#*****************************************************************************

CC=gcc

CFLAGS =
CFLAGS +=-I/usr/local/include/simavr
CFLAGS +=-I/usr/local/include/simavr/avr

ODIR=obj

LIBS=-lsimavr -lelf 

LFLAGS=-L/usr/local/lib

_OBJ = simavr.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: simavr

simavr: $(OBJ)
	gcc -o $@ $^ $(LFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
