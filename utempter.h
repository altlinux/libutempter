
/*
  Copyright (C) 2001,2002  Dmitry V. Levin <ldv@altlinux.org>

  Interface for utempter library.

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef UTEMPTER_H
#define UTEMPTER_H

#ifdef	__cplusplus
extern  "C" {
#endif

/* New interface. */

extern int utempter_add_record (int master_fd, const char *hostname);
extern int utempter_remove_record (int master_fd);
extern int utempter_remove_added_record (void);
extern void utempter_set_helper (const char *pathname);

/* Old interface. */

extern void addToUtmp (const char *pty, const char *hostname, int master_fd);
extern void removeFromUtmp (void);
extern void removeLineFromUtmp (const char *pty, int master_fd);

#ifdef	__cplusplus
}
#endif

#endif /* UTEMPTER_H */
