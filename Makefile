#
# $Id$
# Copyright (C) 2001  Dmitry V. Levin <ldv@fandra.org>
# 
# Makefile for the utempter project
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

PROJECT = utempter
MAJOR = 1
VERSION = $(MAJOR).0.0

SHAREDLIB = lib$(PROJECT).so
SONAME = $(SHAREDLIB).$(MAJOR)
STATICLIB = lib$(PROJECT).a

CFLAGS = $(RPM_OPT_FLAGS)

TARGETS = $(PROJECT) $(SHAREDLIB) $(STATICLIB)

INSTALL = install

all: $(TARGETS)

%.os: %.c
	$(COMPILE.c) -fPIC $< $(OUTPUT_OPTION)

$(SHAREDLIB): iface.os
	$(LINK.o) -shared -Wl,-soname,$(SONAME) -lc $+ $(OUTPUT_OPTION)

$(STATICLIB): iface.o
	$(AR) $(ARFLAGS) $@ $+
	-ranlib $@

iface.o: iface.c utempter.h

install:
	$(INSTALL) -D -m2711 $(PROJECT) $(libdir)/$(PROJECT)/$(PROJECT)
	$(INSTALL) -D -m644 $(PROJECT).h $(includedir)/$(PROJECT).h
	$(INSTALL) -D -m644 $(SHAREDLIB) $(libdir)/$(SHAREDLIB).$(VERSION)
	$(INSTALL) -D -m644 $(STATICLIB) $(libdir)/$(STATICLIB)
	ln -s $(SHAREDLIB).$(VERSION) $(libdir)/$(SHAREDLIB).$(MAJOR)
	ln -s $(SHAREDLIB).$(MAJOR) $(libdir)/$(SHAREDLIB)

clean:
	$(RM) $(TARGETS) iface.o iface.os
