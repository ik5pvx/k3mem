/* ----------------------------------------------------------------------------
 *
 * k3comms.c
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdlib.h>

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

int open_ser(char *device, int speed)
{
	int fd;
	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0)
		return fd;

	if (configure_ser(fd, speed) < 0) {
		close(fd);
		fd = -1;
		goto out;
	}

out:
	return (fd);
}

int setMemChannel(int fd, int memNum)
{
	char cmd[8];
	int err = 0;
	size_t cmdlen = 0;
	ssize_t writelen = 0;

	sprintf(cmd, "MC%03d;", memNum);
	cmdlen = strlen(cmd);
	writelen = write(fd, cmd, cmdlen);
	if (writelen != cmdlen)
		err = -1;

	/*
	 * do we really need this?
	 * k3mem exits right away and nothing happens after this
	 */
	usleep(600e3);

	return err;
}

char *k3Command(int fd, char *cmd, int msecSleep, int bytesToRead)
{

	char *response;
	int i, n;

	response = malloc(200);
	write(fd, cmd, strlen(cmd));
	usleep(msecSleep*1000);

	for (i = 0; i < bytesToRead; i++) {
		do { 
			n = read(fd, response + i, 1); 
			usleep(100); 
			/* FIXME: intercept ";" and exit the loop */
		} while (n != 1);
	}

	return response;
}
