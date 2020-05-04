/*
  Copyright (C) 2001-2005  Dmitry V. Levin <ldv@altlinux.org>

  Utempter library interface.

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
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "utempter.h"
#include "diag.h"

#ifndef TEMP_FAILURE_RETRY
# define TEMP_FAILURE_RETRY(expression) \
  (__extension__							      \
    ({ long int __result;						      \
       do __result = (long int) (expression);				      \
       while (__result == -1L && errno == EINTR);			      \
       __result; }))
#endif

#define UTEMPTER_DEFAULT_PATHNAME	LIBEXECDIR "/utempter/utempter"

static const char *utempter_pathname;
static int saved_fd = -1;

static void __attribute__((__noreturn__))
do_child(int master_fd, const char *path, char *const *argv)
{
	if ((dup2(master_fd, STDIN_FILENO) != STDIN_FILENO)
	    || (dup2(master_fd, STDOUT_FILENO) != STDOUT_FILENO)) {
		print_dbg("dup2: %s", strerror(errno));
		_exit(EXIT_FAILURE);
	}

	execv(path, argv);
	print_dbg("execv: %s", strerror(errno));

	while (EACCES == errno) {
		/* try saved group ID */
		gid_t rgid, egid, sgid;

		if (getresgid(&rgid, &egid, &sgid))
			break;

		if (sgid == egid)
			break;

		if (setgid(sgid))
			break;

		(void) execv(path, argv);
		break;
	}

	_exit(EXIT_FAILURE);
}

static int
execute_helper(int master_fd, const char *const argv[])
{
	struct sigaction saved_action, action;

	action.sa_handler = SIG_DFL;
	action.sa_flags = SA_RESTART;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGCHLD, &action, &saved_action) < 0) {
		print_dbg("sigaction: %s", strerror(errno));
		return 0;
	} else {
		pid_t child;
		int status = 1;

		child = fork();
		if (!child) {
			do_child(master_fd, argv[0], (char *const *) argv);
		} else if (child < 0) {
			print_dbg("fork: %s", strerror(errno));
			goto ret;
		}

		if (TEMP_FAILURE_RETRY(waitpid(child, &status, 0)) < 0) {
			print_dbg("waitpid: %s", strerror(errno));
			status = 1;
		}

	      ret:
		(void) sigaction(SIGCHLD, &saved_action, 0);
		return !status;
	}
}

/* New interface. */

int
utempter_add_record(int master_fd, const char *hostname)
{
	const char *const args[] = {
		utempter_pathname ? : UTEMPTER_DEFAULT_PATHNAME,
		"add",
		hostname,
		0
	};
	int status = execute_helper(master_fd, args);

	if (status)
		saved_fd = master_fd;

	return status;
}

int
utempter_remove_record(int master_fd)
{
	const char *const args[] = {
		utempter_pathname ? : UTEMPTER_DEFAULT_PATHNAME,
		"del",
		0
	};
	int status = execute_helper(master_fd, args);

	if (master_fd == saved_fd)
		saved_fd = -1;

	return status;
}

int
utempter_remove_added_record(void)
{
	if (saved_fd < 0)
		return 0;
	else
		return utempter_remove_record(saved_fd);
}

void
utempter_set_helper(const char *pathname)
{
	utempter_pathname = pathname;
}

/* Old interface. */

void
addToUtmp(const char *pty __attribute__((unused)),
	  const char *hostname, int master_fd)
{
	(void) utempter_add_record(master_fd, hostname);
}

void
removeFromUtmp(void)
{
	(void) utempter_remove_added_record();
}

void
removeLineFromUtmp(const char *pty __attribute__((unused)), int master_fd)
{
	(void) utempter_remove_record(master_fd);
}
