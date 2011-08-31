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
#include <stdio.h>	/*Standard input/output definitions*/
#include <string.h>	/*String function definitions*/
#include <unistd.h>	/*Unix standard function definitions*/
#include <fcntl.h>	/*File control definitions*/
#include <errno.h>	/*Error number definitions*/
#include <termios.h>	/*POSIX terminal control definitions*/
#include <stdlib.h>
#include <math.h>

#include "k3comms.h"

#define LSB 1
#define USB 2
#define CW 3
#define FM 4
#define AM 5
#define RTTY 6
#define CWREV 7
#define RTTYREV 9

/*
 * Shorthand:
 * ( KN
 * + AR
 * = BT
 * % AS
 * * SK
 *
 * < into test mode
 * > out of test mode
 * @ terminate cw send in progress
 */

struct displayVals {
	char digit[8];
	short decimalPoint[8];
	short annunciator[8];
	short flashStatus[8];
} k2display;

int openPort(char *device)
{
	int fd;

	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	/* fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY); */
	if (fd == -1) {
		char e[256];
		snprintf(e, 256,"openPort: Unable to open %s - ",device);
		perror(e);
	}
	else
		fcntl(fd, F_SETFL, 0);
	return(fd);
}

void configurePort(int portFd, int speed) {
	/*Configure port*/
	struct termios options;

	/*Get the current options for the port*/
	tcgetattr(portFd, &options);

	/*Set the Baud rates to 38400*/
	cfsetispeed(&options, speed);
	cfsetospeed(&options, speed);

	/*Enable received and set local mode*/
	options.c_cflag |= (CLOCAL | CREAD);

	/*Set data bits*/
	options.c_cflag &= ~CSIZE;	/*Mask the character size bits*/
	options.c_cflag |= CS8;		/*Select 8 data bits*/

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

	/*Set RAW input*/
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	/*Set Raw output*/
	options.c_oflag &= ~OPOST;

	/*Set timeout to 10 sec*/
	options.c_cc[VTIME] = 6;
#if 0
	options.c_cc[VMIN] = 0;
#endif

	/* Flush serial port. */
	tcflush(portFd, TCIFLUSH);

	/*Set new options for port*/
	if (tcsetattr(portFd, TCSANOW, &options) == -1) {
		perror("Couldn't change serial port settings.  Quitting.\n");
	}
}

void setMemChannel(int fd, int memNum) {
	char cmd[8];

	sprintf(cmd, "MC%03d;", memNum);
	write(fd, cmd, strlen(cmd));
	usleep(600e3);
}

char * k3Command(int fd, char * cmd, int msecSleep, int bytesToRead) {

	char *response;
	int i, n;

	response = malloc(200);
	write(fd, cmd, strlen(cmd));
	usleep(msecSleep);

	for (i = 0; i < bytesToRead; i++) {
		usleep(msecSleep);
		if ((n = read(fd, response+i, 1)) != 1) {
			printf("got %d bytes, not 1\n", n);
		}
	}

	return response;
}


