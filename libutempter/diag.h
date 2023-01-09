/*
  Copyright (C) 2020  Dmitry V. Levin <ldv@altlinux.org>

  Diagnostic macros for libutempter and its helper.

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

#ifndef UTEMPTER_DIAG_H
# define UTEMPTER_DIAG_H

# include <syslog.h>
# include <unistd.h>

# ifdef UTEMPTER_LOG
#  define log_debug(fmt_, ...) syslog(LOG_DEBUG, fmt_, ##__VA_ARGS__)
#  define log_error(fmt_, ...) syslog(LOG_ERR, fmt_, ##__VA_ARGS__)
# else
#  define log_debug(fmt_, ...) do { ; /* no log_debug */ } while (0)
#  define log_error(fmt_, ...) do { ; /* no log_error */ } while (0)
# endif

# ifdef UTEMPTER_DEBUG
#  define print_dbg(fmt_, ...)				\
	fprintf(stderr, "%s:%d: " fmt_ "\n", __FILE__, __LINE__, ##__VA_ARGS__)
# else
#  define print_dbg(fmt_, ...) do { ; /* no debug */ } while (0)
# endif

# define debug_msg(fmt_, ...)				\
	do {						\
		log_debug(fmt_, ##__VA_ARGS__);		\
		print_dbg(fmt_, ##__VA_ARGS__);		\
	} while (0)					\
/* End of debug_msg definition.  */

# define fatal_error(fmt_, ...)							\
	do {									\
		log_error("[ppid=%ld] " fmt_, (long)getppid(), ##__VA_ARGS__);	\
		print_dbg(fmt_, ##__VA_ARGS__);					\
		exit(EXIT_FAILURE);						\
	} while (0)								\
/* End of fatal_error definition.  */

#endif /* UTEMPTER_DIAG_H */
