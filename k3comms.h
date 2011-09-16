/* ----------------------------------------------------------------------------
 *
 * k3comms.h
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

#ifndef __K3COMMS_H__
#define __K3COMMS_H__

/*
 * device -> path to serial device (ex: /dev/ttyS0)
 * speed  -> B38400/B19200...
 * errbuf -> pointer to a char buffer of at least 255 bytes where to store
 *           errors
 *
 * return fd on success, -1 on error and errno + errbuf are set
 */
int k3open(const char *device, int speed,
	   char *errbuf, size_t errbuf_len);

/*
 * fd         -> serial file descriptor from k3open
 */
int k3close(int fd);

/*
 * fd         -> serial file descriptor from k3open
 * cmd        -> command to send to k3
 * resbuf     -> pointer to a buffer where to store the result
 * resbuf_len -> length of the above buffer
 * readlen    -> expected bytes to read in response and store in resbuf
 *               readlen > 0 expects N bytes (will break at ";")
 *               readlen = 0 unknown bytes to read (will break at ";")
 *               readlen < 0 do not expect an answer back
 *               (NOTE: readlen must be at least one char smaller than resbuf_len)
 *
 * return -1 on error,
 *         0 if no bytes have been read or timeout waiting for data
 *        >0 number of bytes read from the response
 * 
 */
int k3cmd(int fd, const char *cmd,
	  char *resbuf, size_t resbuf_len,
	  int readlen);

#endif /* __K3COMMS_H__ */
