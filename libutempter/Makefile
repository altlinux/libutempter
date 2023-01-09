#
# Copyright (C) 2001-2005,2010  Dmitry V. Levin <ldv@altlinux.org>
#
# Makefile for the libutempter project.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

PROJECT = utempter
VERSION = $(shell sed '/^Version: */!d;s///;q' libutempter.spec)
MAJOR = 0

SHAREDLIB = lib$(PROJECT).so
SONAME = $(SHAREDLIB).$(MAJOR)
STATICLIB = lib$(PROJECT).a
MAP = lib$(PROJECT).map

TARGETS = $(PROJECT) $(SHAREDLIB)

INSTALL = install
libdir = /usr/lib
libexecdir = /usr/lib
includedir = /usr/include
mandir = /usr/share/man
man3dir = $(mandir)/man3
DESTDIR =

WARNINGS = -W -Wall -Waggregate-return -Wcast-align -Wconversion \
	-Wdisabled-optimization -Wformat=2 -Wmissing-declarations \
	-Wmissing-format-attribute -Wmissing-noreturn \
	-Wmissing-prototypes -Wpointer-arith -Wredundant-decls \
	-Wshadow -Wstrict-prototypes -Wundef -Wunused -Wwrite-strings
COMPILE_DIALECT = -std=gnu99
COMPILE_LFS =
COMPILE_PIC = -fPIC
COMPILE_PIE = -fPIE
LINK_PIE = -pie
LINK_NOW = -Wl,-z,now
LINK_RELRO = -Wl,-z,relro
LINK_STATS = -Wl,-stats
override CPPFLAGS := $(COMPILE_DIALECT) $(COMPILE_LFS) -DLIBEXECDIR=\"$(libexecdir)\" $(CPPFLAGS)
override CFLAGS := $(WARNINGS) $(CFLAGS) $(RPM_OPT_FLAGS)
override LDFLAGS := $(LINK_RELRO) $(LINK_STATS) $(LDFLAGS)
LDLIBS =

all: $(TARGETS)

%.os: %.c
	$(COMPILE.c) $(COMPILE_PIC) $< $(OUTPUT_OPTION)

$(PROJECT): utempter.c diag.h
	$(LINK.c) $(COMPILE_PIE) $(LINK_PIE) $(LINK_NOW) $(LDLIBS) $< $(OUTPUT_OPTION)

$(SHAREDLIB): iface.os $(MAP)
	$(LINK.o) -shared \
		-Wl,-soname,$(SONAME),--version-script=$(MAP),-z,defs \
		-lc $< $(OUTPUT_OPTION)

$(STATICLIB): iface.o
	$(AR) $(ARFLAGS) $@ $<
	-ranlib $@

iface.os: iface.c utempter.h diag.h

iface.o: iface.c utempter.h diag.h

install:
	mkdir -p $(DESTDIR)$(libdir) $(DESTDIR)$(libexecdir)/$(PROJECT) \
		$(DESTDIR)$(includedir) $(DESTDIR)$(man3dir)
	$(INSTALL) -p -m2711 $(PROJECT) $(DESTDIR)$(libexecdir)/$(PROJECT)/
	$(INSTALL) -p -m644 $(PROJECT).h $(DESTDIR)$(includedir)/
	$(INSTALL) -p -m755 $(SHAREDLIB) $(DESTDIR)$(libdir)/$(SHAREDLIB).$(VERSION)
	ln -s $(SHAREDLIB).$(VERSION) $(DESTDIR)$(libdir)/$(SONAME)
	ln -s $(SONAME) $(DESTDIR)$(libdir)/$(SHAREDLIB)
	$(INSTALL) -p -m644 $(PROJECT).3 $(DESTDIR)$(man3dir)/
	for n in lib$(PROJECT) utempter_add_record utempter_remove_record \
	    utempter_remove_added_record utempter_set_helper; do \
		ln -s $(PROJECT).3 $(DESTDIR)$(man3dir)/$$n.3; \
	done

clean:
	$(RM) $(TARGETS) iface.o iface.os core *~
