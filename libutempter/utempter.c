
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
#include <fcntl.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <utmp.h>

#ifdef __GLIBC__
# include <pty.h>
#elif defined(__FreeBSD__)
# include <libutil.h>
#else
# error Unsupported platform
#endif /* __GLIBC__ || __FreeBSD__ */

#define	DEV_PREFIX	"/dev/"
#define	DEV_PREFIX_LEN	(sizeof(DEV_PREFIX)-1)

static void __attribute__ ((__noreturn__))
usage(void)
{
#ifdef	UTEMPTER_DEBUG
	fprintf(stderr, "Usage: utempter add [<host>]\n"
		"       utempter del\n");
#endif
	exit(EXIT_FAILURE);
}

static void
validate_device(const char *device)
{
	int     flags;
	struct stat stb;

	if (strncmp(device, DEV_PREFIX, DEV_PREFIX_LEN))
	{
#ifdef	UTEMPTER_DEBUG
		fprintf(stderr, "utempter: invalid device name\n");
#endif
		exit(EXIT_FAILURE);
	}

	if ((flags = fcntl(STDIN_FILENO, F_GETFL, 0)) < 0)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf(stderr, "utempter: fcntl: %s\n", strerror(errno));
#endif
		exit(EXIT_FAILURE);
	}

	if ((flags & O_RDWR) != O_RDWR)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf(stderr, "utempter: invalid descriptor mode\n");
#endif
		exit(EXIT_FAILURE);
	}

	if (stat(device, &stb) < 0)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf(stderr, "utempter: %s: %s\n", device,
			strerror(errno));
#endif
		exit(EXIT_FAILURE);
	}

	if (getuid() != stb.st_uid)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf(stderr, "utempter: %s belongs to another user\n",
			device);
#endif
		exit(EXIT_FAILURE);
	}
}

static int
write_uwtmp_record(const char *user, const char *term, const char *host,
#ifdef __GLIBC__
		   pid_t pid,
#endif
		   int add)
{
	struct utmp ut;
	struct timeval tv;

#ifdef __GLIBC__
	size_t offset;
#endif

	memset(&ut, 0, sizeof(ut));

	memset(&tv, 0, sizeof(tv));
	(void) gettimeofday(&tv, 0);

	strncpy(ut.ut_name, user, sizeof(ut.ut_name));
	strncpy(ut.ut_line, term, sizeof(ut.ut_line));
	if (host)
		strncpy(ut.ut_host, host, sizeof(ut.ut_host));

#ifdef __GLIBC__

	offset = (strlen(term) <= sizeof(ut.ut_id)) ? 0 :
			strlen(term) - sizeof(ut.ut_id);
	strncpy(ut.ut_id, term + offset, sizeof(ut.ut_id));

	if (add)
		ut.ut_type = USER_PROCESS;
	else
		ut.ut_type = DEAD_PROCESS;

	ut.ut_pid = pid;

	ut.ut_tv.tv_sec = tv.tv_sec;
	ut.ut_tv.tv_usec = tv.tv_usec;

	setutent();
	if (!pututline(&ut))
	{
#ifdef	UTEMPTER_DEBUG
		fprintf(stderr, "utempter: pututline: %s\n", strerror(errno));
#endif
		exit(EXIT_FAILURE);
	}
	endutent();

	(void) updwtmp(_PATH_WTMP, &ut);

#elif defined(__FreeBSD__)

	ut.ut_time = tv.tv_sec;

	if (add)
	{
		login(&ut);
	} else
	{
		if (logout(term) != 1)
		{
#ifdef	UTEMPTER_DEBUG
			fprintf(stderr, "utempter: logout: %s\n",
				strerror(errno));
#endif
			exit(EXIT_FAILURE);
		}
	}

#endif /* __GLIBC__ || __FreeBSD__ */

#ifdef	UTEMPTER_DEBUG
	fprintf(stderr,
		"utempter: DEBUG: utmp/wtmp record %s for terminal '%s'\n",
		add ? "added" : "removed", term);
#endif
	return EXIT_SUCCESS;
}

int
main(int argc, const char *argv[])
{
	const char *device, *host;
	struct passwd *pw;
	pid_t   pid;
	int     add = 0, i;

	for (i = 0; i <= 2; ++i)
	{
		struct stat sb;

		if (fstat(i, &sb) < 0)
			/* At this stage, we shouldn't even report error. */
			exit(EXIT_FAILURE);
	}

	if (argc < 2)
		usage();

	if (!strcmp(argv[1], "add"))
	{
		if (argc > 3)
			usage();
		add = 1;
	} else if (!strcmp(argv[1], "del"))
	{
		if (argc != 2)
			usage();
		add = 0;
	} else
		usage();

	host = argv[2];

	pid = getppid();
	if (pid == 1)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf(stderr,
			"utempter: parent process should not be init\n");
#endif
		exit(EXIT_FAILURE);
	}

	pw = getpwuid(getuid());
	if (!pw || !pw->pw_name)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf(stderr,
			"utempter: cannot find valid user with uid=%u\n",
			getuid());
#endif
		exit(EXIT_FAILURE);
	}

	device = ptsname(STDIN_FILENO);

	if (!device)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf(stderr, "utempter: cannot find slave pty: %s\n",
			strerror(errno));
#endif
		exit(EXIT_FAILURE);
	}

	validate_device(device);

	return write_uwtmp_record(pw->pw_name, device + DEV_PREFIX_LEN, host,
#ifdef __GLIBC__
				  pid,
#endif
				  add);
}
