#
# $Id$
# Copyright (C) 2001-2004  Dmitry V. Levin <ldv@altlinux.org>
# 
# Makefile for the utempter project
#
# This file is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
#

PROJECT = utempter
VERSION = $(shell grep ^Version: libutempter.spec |head -1 |awk '{print $$2}')
MAJOR = 0

SHAREDLIB = lib$(PROJECT).so
SONAME = $(SHAREDLIB).$(MAJOR)
STATICLIB = lib$(PROJECT).a
MAP = lib$(PROJECT).map

TARGETS = $(PROJECT) $(SHAREDLIB) $(STATICLIB)

INSTALL = install
libdir = /usr/lib
libexecdir = /usr/lib
includedir = /usr/include
DESTDIR =

CFLAGS = $(RPM_OPT_FLAGS) -DLIBEXECDIR=\"$(libexecdir)\"

all: $(TARGETS)

%.os: %.c
	$(COMPILE.c) -fPIC $< $(OUTPUT_OPTION)

$(SHAREDLIB): iface.os $(MAP)
	$(LINK.o) -shared -Wl,-soname,$(SONAME) -Wl,--version-script=$(MAP) -lc $< $(OUTPUT_OPTION)

$(STATICLIB): iface.o
	$(AR) $(ARFLAGS) $@ $<
	-ranlib $@

iface.o: iface.c utempter.h

install:
	$(INSTALL) -pD -m2711 $(PROJECT) $(DESTDIR)$(libexecdir)/$(PROJECT)/$(PROJECT)
	$(INSTALL) -pD -m644 $(PROJECT).h $(DESTDIR)$(includedir)/$(PROJECT).h
	$(INSTALL) -pD -m755 $(SHAREDLIB) $(DESTDIR)$(libdir)/$(SHAREDLIB).$(VERSION)
	$(INSTALL) -pD -m644 $(STATICLIB) $(DESTDIR)$(libdir)/$(STATICLIB)
	ln -s $(SHAREDLIB).$(VERSION) $(DESTDIR)$(libdir)/$(SONAME)
	ln -s $(SONAME) $(DESTDIR)$(libdir)/$(SHAREDLIB)

clean:
	$(RM) $(TARGETS) iface.o iface.os core *~
