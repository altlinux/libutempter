
/*
  $Id$

  Copyright (C) 2001,2002  Dmitry V. Levin <ldv@altlinux.org>

  Utempter library interface.

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
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "utempter.h"

#define	UTEMPTER_DEFAULT_PATHNAME	LIBEXECDIR "/utempter/utempter"

static const char *utempter_pathname;
static int saved_fd = -1;

static int
execute_helper (int master_fd, const char *const argv[])
{
	sigset_t new_set, old_set;

	sigemptyset (&new_set);
	sigaddset (&new_set, SIGCHLD);

	if (sigprocmask (SIG_BLOCK, &new_set, &old_set) < 0)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf (stderr, "libutempter: sigprocmask: %s\n",
			 strerror (errno));
#endif
		return 0;
	} else
	{
		pid_t   child;
		int     status = 1;

		child = fork ();
		if (!child)
		{
			/* child */
			if ((dup2 (master_fd, STDIN_FILENO) != STDIN_FILENO)
			    || (dup2 (master_fd, STDOUT_FILENO) !=
				STDOUT_FILENO))
			{
#ifdef	UTEMPTER_DEBUG
				fprintf (stderr, "libutempter: dup2: %s\n",
					 strerror (errno));
#endif
				_exit (EXIT_FAILURE);
			}
			execv (argv[0], (char *const *) argv);
#ifdef	UTEMPTER_DEBUG
			fprintf (stderr, "libutempter: execv: %s\n",
				 strerror (errno));
#endif
			_exit (EXIT_FAILURE);
		} else if (child < 0)
		{
#ifdef	UTEMPTER_DEBUG
			fprintf (stderr, "libutempter: fork: %s\n",
				 strerror (errno));
#endif
			goto ret;
		}

		if (TEMP_FAILURE_RETRY (waitpid (child, &status, 0)) < 0)
		{
#ifdef	UTEMPTER_DEBUG
			fprintf (stderr, "libutempter: waitpid: %s\n",
				 strerror (errno));
#endif
			status = 1;
		}

		ret:
		if (!sigismember (&old_set, SIGCHLD))
			sigprocmask (SIG_UNBLOCK, &new_set, 0);
		return !status;
	}
}

int
utempter_add_record (int master_fd, const char *hostname)
{
	const char *const args[] = { utempter_pathname ?: UTEMPTER_DEFAULT_PATHNAME, "add", hostname, 0 };
	int     status = execute_helper (master_fd, args);

	if (status)
		saved_fd = master_fd;

	return status;
}

int
utempter_remove_record (int master_fd)
{
	const char *const args[] = { utempter_pathname ?: UTEMPTER_DEFAULT_PATHNAME, "del", 0 };
	int     status = execute_helper (master_fd, args);

	if (master_fd == saved_fd)
		saved_fd = -1;

	return status;
}

int
utempter_remove_added_record (void)
{
	if (saved_fd < 0)
		return 0;
	else
		return utempter_remove_record (saved_fd);
}

int
utempter_set_helper (const char *pathname)
{
	utempter_pathname = UTEMPTER_DEFAULT_PATHNAME;
}
