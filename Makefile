#
# $Id$
# Copyright (C) 2001,2002  Dmitry V. Levin <ldv@altlinux.org>
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
MAJOR = $(shell echo $(VERSION) |cut -d. -f1)

SHAREDLIB = lib$(PROJECT).so
SONAME = $(SHAREDLIB).$(MAJOR)
SONAME_COMPAT = $(SHAREDLIB).0
STATICLIB = lib$(PROJECT).a

TARGETS = $(PROJECT) $(SHAREDLIB) $(STATICLIB) $(SONAME_COMPAT)

INSTALL = install
libdir = /usr/lib
libexecdir = /usr/lib
includedir = /usr/include

CFLAGS = $(RPM_OPT_FLAGS) -DLIBEXECDIR=\"$(libexecdir)\"

all: $(TARGETS)

%.os: %.c
	$(COMPILE.c) -fPIC $< $(OUTPUT_OPTION)

$(SHAREDLIB): iface.os
	$(LINK.o) -shared -Wl,-soname,$(SONAME) -lc $+ $(OUTPUT_OPTION)

$(SONAME_COMPAT): iface0.os $(SHAREDLIB)
	$(LINK.o) -shared -Wl,-soname,$(SONAME_COMPAT) -lc -L. -l$(PROJECT) $+ $(OUTPUT_OPTION)

$(STATICLIB): iface.o
	$(AR) $(ARFLAGS) $@ $+
	-ranlib $@

iface.o: iface.c utempter.h

install:
	$(INSTALL) -D -m2711 $(PROJECT) $(libexecdir)/$(PROJECT)/$(PROJECT)
	$(INSTALL) -D -m644 $(PROJECT).h $(includedir)/$(PROJECT).h
	$(INSTALL) -D -m755 $(SHAREDLIB) $(libdir)/$(SHAREDLIB).$(VERSION)
	$(INSTALL) -D -m755 $(SONAME_COMPAT) $(libdir)/$(SONAME_COMPAT)
	$(INSTALL) -D -m644 $(STATICLIB) $(libdir)/$(STATICLIB)
	ln -s $(SHAREDLIB).$(VERSION) $(libdir)/$(SHAREDLIB).$(MAJOR)
	ln -s $(SHAREDLIB).$(MAJOR) $(libdir)/$(SHAREDLIB)

clean:
	$(RM) $(TARGETS) iface.o iface.os iface0.os
