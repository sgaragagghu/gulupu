#/*
# * Copyright (C) 2021 sgaragagghu marco.scarlino@students-live.uniroma2.it
# *
# * This file is part of Gulupu
# * Gulupu is free software; you can redistribute it and/or
# * modify it under the terms of the GNU General Public
# * License as published by the Free Software Foundation; either
# * version 3 of the License, or (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# * General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this library. If not, see <http://www.gnu.org/licenses/>.
# */

CC ?= gcc
WINDRES = windres.exe
PKGCONFIG = $(shell which pkg-config)
CFLAGS = $(shell $(PKGCONFIG) --cflags gtk+-3.0)
LIBS = $(shell $(PKGCONFIG) --libs gtk+-3.0)

ifdef OS
 OS = windows
else
 OS = unix
endif

FLAGS = -std=gnu11 -O3 -Wall -Wmissing-prototypes -Wno-unused-variable

SRC =						\
	./gui/main.c			\
	./gui/app.c				\
	./gui/appwin.c			\
	./logic/parser_thread.c \
	./logic/config.c		\

# Windows resource -> exe properties as icon, version... etc
WRSC = ./resource/resource.rc

RSC_XML = ./resource/.gresource.xml

# GTK resources as for example templates
OBJS = $(SRC:.c=.o)

ifeq ($(OS), windows)
 OBJ_WRSC = $(WRSC:.rc=.o)
endif

RSC_SRC = $(RSC_XML:.xml=.c)
RSC_OBJ = $(RSC_SRC:.c=.o)

all: gulupu

%.o: %.rc
	$(WINDRES) $< $@

%.c: %.xml
	glib-compile-resources $< --target=$@ --generate-source

%.o: %.c
	$(CC) -c -o $@ $(FLAGS) $(CFLAGS) $<

gulupu: $(RSC_SRC) $(RSC_OBJ) $(OBJS) $(OBJ_WRSC)
	$(CC) -o $(@F) $(OBJS) $(OBJ_WRSC) $(RSC_OBJ) $(LIBS)

clean:
	rm -f $(OBJS) $(OBJ_WRSC) $(RSC_SRC) $(RSC_OBJ)
	rm -f gulupu
