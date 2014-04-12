# Makefile for seekablepipe
# Copyright 2014 Eric Smith <spacewar@gmail.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# ------------------------------------------------------------------
# The following could be overridden on the make command line,
# for instance, for package systems such as RPM.  DESTDIR is
# used when installing into a packaging buildroot.

PREFIX = /usr/local
MANDIR = $(PREFIX)/share/man
DESTDIR =

# For debug:
OPTFLAGS = -g
# For production:
# OPTFLAGS = -O2

# ------------------------------------------------------------------

CFLAGS = -Wall -Wextra $(OPTFLAGS)
LDFLAGS = $(CFLAGS)

TARGETS = seekablepipe

all: $(TARGETS)

seekablepipe: seekablepipe.o

seekablepipe.o: seekablepipe.c

install:
	install -m 755 seekablepipe $(DESTDIR)$(PREFIX)/bin/
	install -m 644 seekablepipe.1 $(DESTDIR)$(MANDIR)/man1/

clean:
	rm -f $(TARGETS) *.o
