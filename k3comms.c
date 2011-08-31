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

#if 0
static void abSwap(int fd) {
	char cmd[7];

	sprintf(cmd, "SWT11;");
	write(fd, cmd, strlen(cmd));
	usleep(100e3);
}

static int getKeyerSpeed(int fd) {
	char response[6];

	write(fd, "KS;", 3);
	usleep(100e3);
	read(fd, response, 6);
	return atoi(response+2);
}

static void setKeyerSpeed(int fd, int speed) {
	char cmd[8];

	if (speed < 9 || speed > 50)
		return;

	sprintf(cmd, "KS%03d;", speed);
	write(fd, cmd, 6);
	usleep(100e3);
}

void waitWhileBusy(int fd) {
	char response[6];

	do {
		write(fd, "ID;", 3);
		usleep(100e3);
		read(fd, response, 6);
		sleep(20000);
	} while (response[0] == '?');
}

static void sendCW(int fd, char *text) {
	char cwCmd[29];
	char buffer[25];
	int len = strlen(text);
	/*
	 * The K2's keyboard send command, KY, can only send at most 24
	 * characters at a time.  Therefore, longer text is broken into multiple
	 * KY commands.
	 */
	while (len > 0) {
		if (len <= 24) {
			/* Final partial buffer. */
			strcpy(buffer, text);
			len = 0;
		} else {
			/* Full block of 24 characters. */
			strncpy(buffer, text, 24);
			buffer[24] = '\0';
			text += 24;
			len -= 24;
		}
		/* Create K2 KY command. */
		sprintf(cwCmd, "KY %s;", buffer);
		/* Send the character buffer. */
		write(fd, cwCmd, strlen(cwCmd));
		usleep(100e3);
	}
}

static void setExtendedMode(int fd) {
	write(fd, "K31;", 4);
	usleep(100e3);
}

static void setK2ExtendedMode(int fd) {
	write(fd, "K22;", 4);
	usleep(100e3);
}

static int getFreq(int fd, char vfo) {
	char cmd[4], response[15];
	int bytes, freq;

	sprintf(cmd, "F%c;", vfo);
	write(fd, cmd, strlen(cmd));
	usleep(600e3);
	bytes = read(fd, response, 14);
	response[14] = '\0';
	freq = atoi(response+2);
	return freq;
}

static void setMode(int fd, int opMode) {
	char cmd[8];

	sprintf(cmd, "MD%d;", opMode);
	write(fd, cmd, strlen(cmd));
	usleep(100e3);
}

static int determineMode(int hz) {
	int mode;

	hz /= 1000;
	if (hz == 3357 || hz == 5790) /* MARS cw net freqs */
		mode = CW;
	else if (hz >= 520 && hz <= 1610)
		mode = AM;
	else if (hz >= 1800 && hz <= 2000)
		mode = CW;
	else if (hz >= 3500 && hz <= 3600)
		mode = CW;
	else if (hz >= 3600 && hz <= 4000)
		mode = LSB;
	else if (hz >= 5300 && hz <= 5405)
		mode = USB;
	else if (hz >= 7000 && hz <= 7125)
		mode = CW;
	else if (hz >= 7125 && hz <= 7300)
		mode = LSB;
	else if (hz >= 10100 && hz <= 10150)
		mode = CW;
	else if (hz >= 14000 && hz <= 14150)
		mode = CW;
	else if (hz >= 14150 && hz <= 14350)
		mode = USB;
	else if (hz >= 18068 && hz <= 18110)
		mode = CW;
	else if (hz >= 18110 && hz <= 18168)
		mode = USB;
	else if (hz >= 21000 && hz <= 21200)
		mode = CW;
	else if (hz >= 21200 && hz <= 21450)
		mode = USB;
	else if (hz >= 24890 && hz <= 24930)
		mode = CW;
	else if (hz >= 24930 && hz <= 24990)
		mode = USB;
	else if (hz >= 28000 && hz <= 28300)
		mode = CW;
	else if (hz >= 28300 && hz <= 29700)
		mode = USB;
	else if (hz >= 144000 && hz <= 144100)
		mode = CW;
	else if (hz >= 144100 && hz <= 144275)
		mode = USB;
	else if (hz >= 144275 && hz <= 148000)
		mode = FM;
	else {
		printf("Frequency outside of hambands, using USB.\n");
		mode = USB;
	}
	return mode;
}

static void setFreq(int fd, char vfo, int hz) {
	char cmd[15];

	sprintf(cmd, "F%c%011d;", vfo, hz);
	write(fd, cmd, strlen(cmd));
	usleep(100e3);

	/* Adjust mode based on frequency. */
	setMode(fd, determineMode(hz));
}

static void quitCw(int fd) {
	char cmd[15];

	/* sprintf(cmd, "KY @;"); */
	sprintf(cmd, "RX;");
	write(fd, cmd, strlen(cmd));
	usleep(100e3);
}

static void rate(int fd) {
	char cmd[6];

	sprintf(cmd, "SW07;");
	write(fd, cmd, strlen(cmd));
	usleep(100e3);
}

static double getPower(int fd) {
	char cmd[4], response[8];

	sprintf(cmd, "PC;");
	write(fd, cmd, strlen(cmd));
	usleep(100e3);
	read(fd, response, 6);
	if (response[0] != 'P' || response[1] != 'C') {
		printf("Didn't get a proper reply to 'power' command.\n");
		return 0.;
	}
	response[5] = '\0';
	return atof(response+2);
}

static void setPower(int fd, float watts) {
	char cmd[9];

	if (watts <= 12)
		sprintf(cmd, "PC%03d0;", (int) (watts * 10));
	else
		sprintf(cmd, "PC%03d1;", (int) watts);
	setK2ExtendedMode(fd);
	write(fd, cmd, strlen(cmd));
	setExtendedMode(fd);

/*

	level = (watts > 15) ? '1' : '0';
	if (watts <= 15)
		iWatts = (int) (watts * 10);
	else
		iWatts = (int) watts;

	sprintf(cmd, "PC%03d%c;", iWatts, level);
	write(fd, cmd, strlen(cmd));
*/

}

static int getPreamp(int fd) {
	char cmd[8], response[8];

	sprintf(cmd, "PA;");
	write(fd, cmd, strlen(cmd));
	read(fd, response, 4);
	return response[0] == '1';
}

static void setPreamp(int fd, int on) {
	char cmd[8];

	if (on < 0 || on > 1)
		return;

	sprintf(cmd, "PA%d;", on);
	write(fd, cmd, 4);
	usleep(100e3);
}

static int getAttenuator(int fd) {
	char cmd[8], response[8];

	sprintf(cmd, "RA;");
	write(fd, cmd, strlen(cmd));
	read(fd, response, 5);
	return response[1] == '1';
}

static void setAttenuator(int fd, int on) {
	char cmd[8];

	if (on < 0 || on > 1)
		return;

	sprintf(cmd, "RA0%d;", on);
	write(fd, cmd, 5);
	usleep(100e3);
}

static void setSplit(int fd, int hz, int hzUp) {
	char cmd[4];

	sprintf(cmd, "FR0;"); // Set SPLIT mode A-recv, B-xmt.
	write(fd, cmd, strlen(cmd));
	usleep(100e3);
	setFreq(fd, 'A', hz);
	if (hzUp == 0) {
		return;
	}
	sprintf(cmd, "FT1;"); // Set SPLIT mode A-recv, B-xmt.
	write(fd, cmd, strlen(cmd));
	usleep(100e3);
	setFreq(fd, 'B', hz + hzUp);
}

static void getDisplay(int fd) {
	char cmd[4], response[14];
	int i;

	sprintf(cmd, "DS;");
	write(fd, cmd, strlen(cmd));
	usleep(100e3);
	read(fd, response, 13);
	for (i = 0; i < 8; i++) {
		// Strip MSB (decimal point data) from byte.
		k2display.digit[i] = response[i+2] & 0x7f;
		switch (k2display.digit[i]) {
		case '<':
			k2display.digit[i] = 'L';
			break;
		case '>':
			k2display.digit[i] = '-';
			break;
		case '@':
			k2display.digit[i] = ' ';
			break;
		case 'K':
			k2display.digit[i] = 'H';
			break;
		case 'M':
			k2display.digit[i] = 'N';
			break;
		case 'Q':
			k2display.digit[i] = 'O';
			break;
		case 'V':
			k2display.digit[i] = 'U';
			break;
		case 'W':
			k2display.digit[i] = 'I';
			break;
		case 'X':
			k2display.digit[i] = 'c';
			break;
		case 'Z':
			k2display.digit[i] = 'C';
			break;
		case '[':
			k2display.digit[i] = 'r';
			break;
		}
		// Save decimal point on/off data (MSB).
		k2display.decimalPoint[i] = (response[i+2] & 0x80) != 0;
	}
	for (i = 0; i < 8; i++)
		k2display.annunciator[i] = (response[10] & (1 << i)) != 0;
	for (i = 0; i < 8; i++)
		k2display.flashStatus[i] = (response[11] & (1 << i)) != 0;
}

static void tune(int fd) {
	char cmd[7];

	sprintf(cmd, "SWT19;");
	write(fd, cmd, strlen(cmd));
	usleep(100e3);
}

void usage(char * prog) {
	/* Aa:Bb:c:fk:Kl:p:Pqs:tu: */
	printf("Usage: %s\t[-A] get VFO A freq in kHz\n", prog);
	printf("\t\t[-a khz] set VFO A freq in kHz\n");
	printf("\t\t[-B] get VFO B freq in kHz\n");
	printf("\t\t[-b khz] set VFO B freq in kHz\n");
	printf("\t\t[-c 'cw message'] send message within quotes in CW\n");
	printf("\t\t[-f] flip (swap) VFO A and B frequencies\n");
	printf("\t\t[-k wpm] set keyer speed in wpm\n");
	printf("\t\t[-K] get keyer speed in wpm\n");
	printf("\t\t[-p watts] set transmit power in Watts\n");
	printf("\t\t[-P] get transmit power in Watts\n");
	printf("\t\t[-q] terminate CW transmission in progress\n");
	printf("\t\t[-s freq up] go split rx on 'freq' kHz,\n");
	printf("\t\t             tx on 'freq' + 'up' kHz\n");
	printf("\t\t[-t] low power ATU tune\n");
}

int main(int argc, char *argv[])
{
	char c;
	char *cwString, *levelString;
	double freq, freqB, power, split;
	int fd;
	int cwCmd;
	int doGetFreq;
	int doSetFreq;
	int doGetFreqB;
	int doSetFreqB;
	int doGetKeyerSpeed;
	int doQuit;
	int doSetKeyerSpeed;
	int flipFreq;
	int levelCmd;
	int powerCmd;
	int doGetPower;
	int setSpeed;
	int setASplit;
	int speed;
	int tuneUp;

	speed = 25;
	cwCmd = doGetFreq = doSetFreq = doSetFreqB = doGetFreqB = levelCmd
		= flipFreq = powerCmd = doGetPower = setSpeed = setASplit = tuneUp
		= doQuit = doGetKeyerSpeed = doSetKeyerSpeed = 0;
	while ((c = getopt(argc, argv, "Aa:Bb:c:fhk:Kl:p:Pqs:t")) != EOF) {
		switch (c) {
		case 'A':
			doGetFreq = 1;
			break;
		case 'a':
			doSetFreq = 1;
			freq = atof(optarg);
			break;
		case 'B':
			doGetFreqB = 1;
			break;
		case 'b':
			doSetFreqB = 1;
			freqB = atof(optarg);
			break;
		case 'c':       /* CW generation. */
			cwCmd = 1;
			cwString = strdup(optarg);
			break;
		case 'f':
			flipFreq = 1;
			break;
		case 'h':
			usage(argv[0]);
			exit(1);
		case 'k':       /* Set keyer speed. */
			doSetKeyerSpeed = 1;
			speed = atoi(optarg);
			break;
		case 'K':       /* Get keyer speed. */
			doGetKeyerSpeed = 1;
			break;
		case 'l':       /* Adjust to receiver noise floor. */
			levelCmd = 1;
			levelString = strdup(optarg);
			break;
		case 'p':       /* Set power. */
			powerCmd = 1;
			power = atof(optarg);
			break;
		case 'P':       /* Get power. */
			doGetPower = 1;
			break;
		case 'q':       /* Immediately stop CW transmission. */
			doQuit = 1;
			break;
		case 't':		/* Tune. */
			tuneUp = 1;
			break;
		case 's':       /* Set split. */
			setASplit = 1;
			freq = atof(optarg);
			if (argv[optind][0] == 'd' || argv[optind][0] == 'D')
				split = -atof(1 + argv[optind]);
			else if (argv[optind][0] == 'u' || argv[optind][0] == 'U')
				split = atof(1 + argv[optind]);
			else
				split = atof(argv[optind]);
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	fd = openPort();
	configurePort(fd);

	setExtendedMode(fd);

	if (setASplit)
		setSplit(fd, (int) round(freq * 1000), round(split * 1000));

	if (doSetFreq)
		setFreq(fd, 'A', (int) round(freq * 1000));

	if (doGetFreq)
		printf("%.3f\n", getFreq(fd, 'A')/1000.);

	if (doSetFreqB)
		setFreq(fd, 'B', (int) round(freq * 1000));

	if (doSetFreqB)
		setFreq(fd, 'B', (int) round(freq * 1000));

	if (doQuit)
		quitCw(fd);

	if (flipFreq)
		abSwap(fd);

	if (levelCmd) {
		if (!strcmp(levelString, "-10")) {
			/* -10 dB */
			setAttenuator(fd, 1);
			setPreamp(fd, 0);
		} else if (!strcmp(levelString, "0")) {
			/* 0 dB */
			setAttenuator(fd, 0);
			setPreamp(fd, 0);
		} else if (!strcmp(levelString, "4")) {
			/* +4 dB */
			setAttenuator(fd, 1);
			setPreamp(fd, 1);
		} else if (!strcmp(levelString, "14")) {
			/* +14 dB */
			setAttenuator(fd, 0);
			setPreamp(fd, 1);
		} else {
			printf("Level options: -10, 0, 4, or 14 dB\n");
		}
	}

	if (doGetKeyerSpeed)
		printf("%d wpm\n", getKeyerSpeed(fd));

	if (doSetKeyerSpeed)
		setKeyerSpeed(fd, speed);

	if (powerCmd) {
		setPower(fd, power);
	}

	if (doGetPower) {
		printf("%.0f W\n", getPower(fd));
	}

	if (tuneUp)
		tune(fd);

	if (cwCmd)
		sendCW(fd, cwString);

--#if 0
	/*
	int oldSpeed = getKeyerSpeed(fd);
	*/
	setKeyerSpeed(fd, speed);
	/* Send each string on command line as cw. */
	for ( ; optind < argc; optind++)
		sendCW(fd, argv[optind]);

	/*
	setKeyerSpeed(oldSpeed);
	*/
--#endif
	close(fd);

}
#endif
