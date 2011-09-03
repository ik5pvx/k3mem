/* ----------------------------------------------------------------------------
 *
 * k3mem.h
 *
 * Copyright (C) xxxx-2011
 *		Mike Markowski, AB3AP
 * Copyright (C) 2011-
 *		Pierfrancesco Caci, IK5PVX
 *
 * This file is part of k3mem
 *
 * k3mem is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * k3mem is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with k3mem.  If not, see <http://www.gnu.org/licenses/>.
 * ----------------------------------------------------------------------------
 */

#ifndef __K3MEM_H__
#define __K3MEM_H__

#define print_debug(fmt, args...)		\
	do {								\
	    if (debug)						\
			printf("DEBUG: " fmt, ##args);\
	} while(0)

#define print_verbose(fmt, args...)     \
	do {								\
		if (verbose)					\
			printf(fmt, ##args);		\
	   } while(0)

#define DEFAULT_DEVICE "/dev/ttyS0"
#define DEFAULT_SPEED  B38400

/* Used in getopt */
int verbose = 0;          /* -v */
int	debug = 0;            /* -D */

#endif /* __K3MEM_H__ */

