
/*
  $Id$

  Copyright (C) 2001  Dmitry V. Levin <ldv@fandra.org>

  Interface for utempter library.

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

#ifndef __UTEMPTER_H__
#define __UTEMPTER_H__

extern int utempter_add_record (int master_fd, const char *hostname);
extern int utempter_remove_record (int master_fd);
extern int utempter_remove_added_record (void);

/* Provide compatibility with old redhat interface. */

#ifndef	WITH_UTEMPTER_COMPAT
static  inline
#endif
void
addToUtmp (const char *pty __attribute ((unused)), const char *hostname,
	   int master_fd)
{
	utempter_add_record (master_fd, hostname);
}

#ifndef	WITH_UTEMPTER_COMPAT
static  inline
#endif
void
removeFromUtmp (void)
{
	utempter_remove_added_record ();
}

#ifndef	WITH_UTEMPTER_COMPAT
static  inline
#endif
void
removeLineFromUtmp (const char *pty __attribute ((unused)), int master_fd)
{
	utempter_remove_record (master_fd);
}

#endif /* __UTEMPTER_H__ */
