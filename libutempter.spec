# $Id$

Name: libutempter
Version: 1.1.0
Release: alt1
%define helperdir %_libexecdir/utempter

Summary: A privileged helper for utmp/wtmp updates
License: LGPL
Group: System/Libraries

Source: %name-%version.tar.bz2

PreReq: shadow-utils
Provides: libutempter.so.1, utempter = 0.5.2
Obsoletes: utempter

%description
This library provides interface for terminal emulators such as
screen and xterm to record user sessions to utmp and wtmp files.

%package devel
Summary: Development environment for utempter
Group: Development/C
Requires: %name = %version-%release
Provides: utempter-devel = %version
Obsoletes: utempter-devel

%package devel-static
Summary: Static utempter library
Group: Development/C
Requires: %name-devel = %version-%release
Provides: utempter-devel-static = %version
Obsoletes: utempter-devel-static

%description devel
This package contains development files required to build
utempter-based software.

%description devel-static
This package contains static library required to build
statically linked utempter-based software.

%prep
%setup -q

%build
%make_build

%install
make install DESTDIR="$RPM_BUILD_ROOT"

%pre
/usr/sbin/groupadd -r -f utmp >/dev/null 2>&1
/usr/sbin/groupadd -r -f utempter >/dev/null 2>&1

%post -p %post_ldconfig
%postun -p %postun_ldconfig

%triggerpostun -- libutempter < 1.1.0
%__ln_s -f libutempter.so.0 %_libdir/libutempter.so.1

%files
%_libdir/*.so.*
%attr(710,root,utempter) %dir %helperdir
%attr(2711,root,utmp) %helperdir/*
%doc README

%files devel
%_libdir/*.so
%_includedir/*

%files devel-static
%_libdir/*.a

%changelog
* Mon Dec 23 2002 Dmitry V. Levin <ldv@altlinux.org> 1.1.0-alt1
- Changed soname back to libutempter.so.0, introduced versioning.

* Tue Sep 24 2002 Dmitry V. Levin <ldv@altlinux.org> 1.0.7-alt1
- If helper execution fails, try saved group ID.

* Tue May 21 2002 Dmitry V. Levin <ldv@altlinux.org> 1.0.6-alt1
- New function: utempter_set_helper.

* Mon Dec 10 2001 Dmitry V. Levin <ldv@alt-linux.org> 1.0.5-alt1
- iface.c: block SIGCHLD instead of redefine signal handler.

* Wed Nov 21 2001 Dmitry V. Levin <ldv@alt-linux.org> 1.0.4-alt1
- utempter.h: do not use "__attribute ((unused))".

* Tue Nov 13 2001 Dmitry V. Levin <ldv@alt-linux.org> 1.0.3-alt1
- Added compatibility declarations to ease upgrade of old applications.
- Added small README file.
- Corrected provides.

* Thu Nov 08 2001 Dmitry V. Levin <ldv@alt-linux.org> 1.0.2-alt1
- Added compatibility library to ease upgrade of old applications.

* Mon Nov 05 2001 Dmitry V. Levin <ldv@alt-linux.org> 1.0.1-alt1
- Indented code a bit (Solar request).

* Mon Oct 15 2001 Dmitry V. Levin <ldv@alt-linux.org> 1.0.0-alt1
- Rewritten the code completely.
- Renamed to libutempter.
- Corrected the package description.
- FHSificated (yes, there are no more %_sbindir/utempter).
- Libificated.

* Fri Oct 12 2001 Dmitry V. Levin <ldv@altlinux.ru> 0.5.2-alt4
- %_libdir/utempter sounds better so use it as helper directory.

* Thu Oct 11 2001 Dmitry V. Levin <ldv@altlinux.ru> 0.5.2-alt3
- Specfile cleanup.
- Owl-compatible changes:
  + added utempter group;
  + utempter binary moved to %_libdir/utempter.d,
    owned by group utempter with 710 permissions.

* Thu Jun 28 2001 Sergie Pugachev <fd_rag@altlinux.ru> 0.5.2-alt1
- new version

* Tue Dec 05 2000 AEN <aen@logic.ru>
- build for RE

* Tue Jul 25 2000 Thierry Vignaud <tvignaud@mandrakesoft.com> 0.5.1-4mdk
- BM

* Fri May 19 2000 Pixel <pixel@mandrakesoft.com> 0.5.1-3mdk
- add -devel
- add soname
- spec helper cleanup

* Sat Apr 08 2000 Christopher Molnar <molnarc@mandrakesoft.com> 0.5.1-2mdk
- changed group

* Tue Oct 26 1999 Chmouel Boudjnah <chmouel@mandrakesoft.com>
- 0.5.1
- fix utmp as group 22.
- strip utempter.
- defattr to root.

* Thu Jun 10 1999 Bernhard Rosenkränzer <bero@mandrakesoft.com>
- Mandrake adaptions

* Fri Jun  4 1999 Jeff Johnson <jbj@redhat.com>
- ignore SIGCHLD while processing utmp.

