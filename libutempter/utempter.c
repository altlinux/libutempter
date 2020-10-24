/*
  Copyright (C) 2001,2002,2005,2010  Dmitry V. Levin <ldv@altlinux.org>

  A privileged helper for utmp/wtmp updates.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _GNU_SOURCE
# define _GNU_SOURCE	1
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <utmp.h>

#include "diag.h"

#define DEV_PREFIX	"/dev/"
#define DEV_PREFIX_LEN	(sizeof(DEV_PREFIX) - 1)

#define MIN(a_, b_) (((a_) < (b_)) ? (a_) : (b_))

static void
validate_device(const char *device)
{
	int flags;
	struct stat stb;

	if (strncmp(device, DEV_PREFIX, DEV_PREFIX_LEN))
		fatal_error("invalid device name");

	if ((flags = fcntl(STDIN_FILENO, F_GETFL, 0)) < 0)
		fatal_error("fcntl: %s", strerror(errno));

	if ((flags & O_RDWR) != O_RDWR)
		fatal_error("invalid descriptor mode");

	if (stat(device, &stb) < 0)
		fatal_error("%s: %s", device, strerror(errno));

	if (getuid() != stb.st_uid)
		fatal_error("%s belongs to another user", device);
}

static void
validate_hostname(const char *host)
{
	for (const char *p = host; p[0]; ++p) {
		unsigned char c = (unsigned char) p[0];

		if (!(p == host ? isgraph : isprint)(c))
			fatal_error("invalid host name (contains %#x)", c);
	}
}

static int
write_uwtmp_record(const char *user, const char *term, const char *host,
		   pid_t pid, int add)
{
	struct utmp ut;
	struct timeval tv;
	size_t len, offset;

	memset(&ut, 0, sizeof(ut));

	memset(&tv, 0, sizeof(tv));
	(void) gettimeofday(&tv, 0);

	len = strlen(user);
	memcpy(ut.ut_name, user, MIN(sizeof(ut.ut_name), len));

	if (host) {
		len = strlen(host);
		memcpy(ut.ut_host, host, MIN(sizeof(ut.ut_host), len));
	}

	len = strlen(term);
	memcpy(ut.ut_line, term, MIN(sizeof(ut.ut_line), len));

	offset = len <= sizeof(ut.ut_id) ? 0 : len - sizeof(ut.ut_id);
	memcpy(ut.ut_id, term + offset, len - offset);

	if (add)
		ut.ut_type = USER_PROCESS;
	else
		ut.ut_type = DEAD_PROCESS;

	ut.ut_pid = pid;

	ut.ut_tv.tv_sec = (__typeof__(ut.ut_tv.tv_sec)) tv.tv_sec;
	ut.ut_tv.tv_usec = (__typeof__(ut.ut_tv.tv_usec)) tv.tv_usec;

	setutent();
	if (!pututline(&ut))
		fatal_error("pututline: %s", strerror(errno));
	endutent();

	(void) updwtmp(_PATH_WTMP, &ut);

	debug_msg("utmp/wtmp record %s for terminal '%s'",
		  add ? "added" : "removed", term);
	return EXIT_SUCCESS;
}

int
main(int argc, const char *argv[])
{
	const char *device, *host;
	struct passwd *pw;
	pid_t pid;
	int add = 0, i;

	for (i = 0; i <= 2; ++i) {
		struct stat sb;

		if (fstat(i, &sb) < 0)
			/* At this stage, we shouldn't even report error. */
			exit(EXIT_FAILURE);
	}

	if (argc < 2)
		fatal_error("usage error");

	if (!strcmp(argv[1], "add")) {
		if (argc > 3)
			fatal_error("usage error");
		add = 1;
	} else if (!strcmp(argv[1], "del")) {
		if (argc != 2)
			fatal_error("usage error");
		add = 0;
	} else
		fatal_error("usage error");

	host = argv[2];

	pid = getppid();
	if (pid == 1)
		fatal_error("parent process should not be init");

	pw = getpwuid(getuid());
	if (!pw || !pw->pw_name)
		fatal_error("cannot find a valid user with uid=%u", getuid());

	device = ptsname(STDIN_FILENO);
	if (!device)
		fatal_error("cannot find slave pty: %s", strerror(errno));

	validate_device(device);
	if (host)
		validate_hostname(host);

	return write_uwtmp_record(pw->pw_name, device + DEV_PREFIX_LEN, host,
				  pid, add);
}
