
/*
  $Id$

  Copyright (C) 2001  Dmitry V. Levin <ldv@fandra.org>

  A privileged helper for utmp/wtmp updates.

  This file is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This file is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*/

#define _GNU_SOURCE	1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pty.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <utmp.h>

#define	DEV_PREFIX	"/dev/"
#define	DEV_PREFIX_LEN	(sizeof(DEV_PREFIX)-1)

static void
usage (void)
{
#ifdef	UTEMPTER_DEBUG
	fprintf (stderr, "Usage: utempter add [<host>]\n"
		 "       utempter del\n");
#endif
	exit (EXIT_FAILURE);
}

static void
validate_device (const char *device)
{
	int     flags;
	struct stat stb;

	if (strncmp (device, DEV_PREFIX, DEV_PREFIX_LEN))
	{
#ifdef	UTEMPTER_DEBUG
		fprintf (stderr, "utempter: invalid device name\n");
#endif
		exit (EXIT_FAILURE);
	}

	if ((flags = fcntl (STDIN_FILENO, F_GETFL, 0)) < 0)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf (stderr, "utempter: fcntl: %s\n", strerror (errno));
#endif
		exit (EXIT_FAILURE);
	}

	if ((flags & O_RDWR) != O_RDWR)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf (stderr, "utempter: invalid descriptor mode\n");
#endif
		exit (EXIT_FAILURE);
	}

	if (stat (device, &stb) < 0)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf (stderr, "utempter: %s: %s\n", device,
			 strerror (errno));
#endif
		exit (EXIT_FAILURE);
	}

	if (getuid () != stb.st_uid)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf (stderr, "utempter: %s belongs to another user\n",
			 device);
#endif
		exit (EXIT_FAILURE);
	}
}

static int
write_uwtmp_record (const char *user, const char *term, const char *host,
		    pid_t pid, int add)
{
	struct utmp ut;
	int     offset = strlen (term) - sizeof (ut.ut_id);

	memset (&ut, 0, sizeof (ut));

	strncpy (ut.ut_user, user, sizeof (ut.ut_user));

	strncpy (ut.ut_line, term, sizeof (ut.ut_line));

	if (offset < 0)
		offset = 0;
	strncpy (ut.ut_id, term + offset, sizeof (ut.ut_id));

	if (host)
		strncpy (ut.ut_host, host, sizeof (ut.ut_host));

	if (add)
		ut.ut_type = USER_PROCESS;
	else
		ut.ut_type = DEAD_PROCESS;

	ut.ut_pid = pid;

	gettimeofday (&ut.ut_tv, 0);

	setutent ();
	if (!pututline (&ut))
	{
#ifdef	UTEMPTER_DEBUG
		fprintf (stderr, "utempter: pututline: %s\n",
			 strerror (errno));
#endif
		exit (EXIT_FAILURE);
	}
	endutent ();

	updwtmp (_PATH_WTMP, &ut);

#ifdef	UTEMPTER_DEBUG
	fprintf (stderr,
		 "utempter: DEBUG: utmp/wtmp record %s for terminal '%s'\n",
		 add ? "added" : "removed", term);
#endif
	return EXIT_SUCCESS;
}

int
main (int argc, const char *argv[])
{
	const char *device, *host;
	struct passwd *pw;
	pid_t   pid;
	int     add = 0, i;

	for (i = 0; i <= 2; ++i)
	{
		struct stat sb;

		if (fstat (i, &sb) < 0)
			/* At this stage, we shouldn't even report error. */
			exit (EXIT_FAILURE);
	}

	if (argc < 2)
		usage ();

	if (!strcmp (argv[1], "add"))
	{
		if (argc > 3)
			usage ();
		add = 1;
	} else if (!strcmp (argv[1], "del"))
	{
		if (argc != 2)
			usage ();
		add = 0;
	} else
		usage ();

	host = argv[2];

	pid = getppid ();
	if (pid == 1)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf (stderr,
			 "utempter: parent process should not be init\n");
#endif
		exit (EXIT_FAILURE);
	}

	pw = getpwuid (getuid ());
	if (!pw || !pw->pw_name)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf (stderr,
			 "utempter: cannot find valid user with uid=%u\n",
			 getuid ());
#endif
		exit (EXIT_FAILURE);
	}

	device = ptsname (STDIN_FILENO);
	if (!device)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf (stderr, "utempter: cannot find slave pty: %s\n",
			 strerror (errno));
#endif
		exit (EXIT_FAILURE);
	}

	validate_device (device);

	return write_uwtmp_record (pw->pw_name, device + DEV_PREFIX_LEN, host,
				   pid, add);
}
