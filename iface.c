
/*
  $Id$

  Copyright (C) 2001  Dmitry V. Levin <ldv@fandra.org>

  Utempter library interface.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#define	UTEMPTER_PATH	"/usr/lib/utempter/utempter"

static int saved_fd = -1;

static int
execute_helper (int master_fd, const char *const argv[])
{
	struct sigaction saved_action, action;

	action.sa_handler = SIG_IGN;
	sigemptyset (&action.sa_mask);
	action.sa_flags = 0;

	if (sigaction (SIGCHLD, &action, &saved_action) < 0)
	{
#ifdef	UTEMPTER_DEBUG
		fprintf (stderr, "libutempter: sigaction: %s\n",
			 strerror (errno));
#endif
		return 0;
	} else
	{
		pid_t   child;
		int     status = 0;

		child = fork ();
		if (!child)
		{
			/* child */
			if ((dup2 (master_fd, STDIN_FILENO) < 0)
			    || (dup2 (master_fd, STDOUT_FILENO) < 0))
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
			sigaction (SIGCHLD, &saved_action, 0);
			return 0;
		}

		if (TEMP_FAILURE_RETRY (waitpid (child, &status, 0)) < 0)
		{
#ifdef	UTEMPTER_DEBUG
			fprintf (stderr, "libutempter: waitpid: %s\n",
				 strerror (errno));
#endif
			status = 1;
		}

		sigaction (SIGCHLD, &saved_action, 0);
		return !status;
	}
}

int
utempter_add_record (int master_fd, const char *hostname)
{
	const char *const args[] = { UTEMPTER_PATH, "add", hostname, 0 };
	int     status = execute_helper (master_fd, args);

	if (status)
		saved_fd = master_fd;

	return status;
}

int
utempter_remove_record (int master_fd)
{
	const char *const args[] = { UTEMPTER_PATH, "del", 0 };
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
