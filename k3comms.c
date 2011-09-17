/* ----------------------------------------------------------------------------
 *
 * k3comms.c
 *
 * Copyright (C) xxxx-2011
 *		Mike Markowski, AB3AP
 * Copyright (C) 2011-
 *		Pierfrancesco Caci, IK5PVX
 * Copyright (C) 2011-
 *		Fabio M. Di Nitto
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/select.h>

#include "k3comms.h"

static int configure_ser(int fd, int speed)
{
	struct termios options;

	/* probe current serial options */
	if (tcgetattr(fd, &options) < 0)
		return -1;

	/* set I/O speed */
	if (cfsetispeed(&options, speed) < 0)
		return -1;

	if (cfsetospeed(&options, speed) < 0)
		return -1;

	/* Enable received and set local mode */
	options.c_cflag |= (CLOCAL | CREAD);

	/* Set data bits */
	options.c_cflag &= ~CSIZE;	/*Mask the character size bits */
	options.c_cflag |= CS8;	/*Select 8 data bits */

#if 0
	/* K3 should have -1- stop bit set, K2 uses 2 */
	/* Set two stop bits. */
	options.c_cflag |= CSTOPB;
#endif

	/* Set no parity bits. */
	options.c_cflag &= ~PARENB;

	/* Set flow control for K2 to none. */
	options.c_cflag &= ~CRTSCTS;
	options.c_iflag &= ~IXON;

	/* Set RAW input */
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	/* Set Raw output */
	options.c_oflag &= ~OPOST;

	/* Set timeout to 10 sec */
	options.c_cc[VTIME] = 6;
#if 0
	options.c_cc[VMIN] = 0;
#endif

	/* Flush serial port. */
	if (tcflush(fd, TCIFLUSH) < 0)
		return -1;

	return tcsetattr(fd, TCSANOW, &options);
}

static int is_k3(int fd)
{
	char response[256];

	if (k3cmd(fd, "K3;\0", response, sizeof(response), 0) <= 0)
		return -1;

	if (strncmp(response, "K3", 2))
		return -2;

	return 0;
}

int k3open(const char *device, int speed,
	   char *errbuf, size_t errbuf_len)
{
	int fd;
	int saveerr;
	int err;

	if ((!device) || (!errbuf) || (errbuf_len <= 255)) {
		errno = EINVAL;
		return -1;
	}

	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
		saveerr = errno;
		snprintf(errbuf, errbuf_len,
			 "Unable to open serial device: %s\n",
			 strerror(saveerr));
		errno = saveerr;
		return -1;
	}

	if (configure_ser(fd, speed) < 0) {
		saveerr = errno;
		close(fd);
		snprintf(errbuf, errbuf_len,
			 "Unable to configure serial device: %s\n",
			 strerror(saveerr));
		errno = saveerr;
		return -1;
	}

	err = is_k3(fd);
	if (err < 0) {
		switch(err) {
			case -2:
				snprintf(errbuf, errbuf_len,
					 "Device connected on %s"
					 "at %d baud is not a K3.\n",
					 device, speed);
				break;
			case -1:
				snprintf(errbuf, errbuf_len,
					 "Unable to determine if device is K3\n");
				break;
			default:

				break;
		}
		close(fd);
		return -1;
	}
	return fd;
}

int k3close(int fd)
{
	return close(fd);
}

int k3cmd(int fd, const char *cmd,
	  char *resbuf, size_t resbuf_len,
	  int readlen)
{
	fd_set read_fds;
	struct timeval tv;
	int err;
	int offset = 0;

	if ((fd < 0) || (!cmd) ||
	    (!resbuf) || (!resbuf_len) ||
	    (readlen+1 >= resbuf_len)) {
		errno = EINVAL;
		return -1;
	}

	/* send command to k3 */
	err = write(fd, cmd, strlen(cmd));

	/* let's keep it simple write for now */
	if ((readlen < 0) || (err < 0))
		return err;

	/* always clean the buffer */
	memset(resbuf, 0, resbuf_len);

	while (1) {
reselect:
		FD_ZERO (&read_fds);
		FD_SET(fd, &read_fds);

		tv.tv_sec = 5;
		tv.tv_usec = 0;

		err = select((fd + 1), &read_fds, 0, 0, &tv);
		/* take the easy way out for now */
		if (err <= 0)
			return err;

		/* do the fancy read here */
		if (FD_ISSET(fd, &read_fds)) {
readmore:
			if (read(fd, resbuf + offset, 1) != 1) {
				if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
					goto reselect;
				return -1;
			}

			if (resbuf[offset] == ';') {
				offset++;
				goto out;
			}

			if ((offset+1 == resbuf_len) ||
			    ((readlen > 0) && (offset == readlen)))
				goto out;

			offset++;
			goto readmore;
		} else {
			return -1;
		}
	}
out:
	return offset;
}
