/* ----------------------------------------------------------------------------
 *
 * morgue.c -- This file contains pieces of code that are not in use 
 *             at the moment but may be useful later. Maybe. 
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

/*  (removed from erCommand.c) */

/*
 * Given a frequency string within an ER response, calculate the encoded
 * frequency.
 */

/* static int calcFreq(char fStr[], char xvtrFlags) {

	int MHz, kHz, kHzX10, HzX100, Hzx10, Hz;
	int freqVal;

	if (!strncmp(fStr, "FF", 2))
		return -1;

	sscanf(fStr, "%2x%2x%2x%2x%2x", &MHz, &kHzX10, &HzX100, &Hzx10, &Hz);

	if (xvtrFlags & 1) // 2m only?
		switch (MHz) {
		case 28: MHz = 144; break;
		case 29: MHz = 145; break;
		case 30: MHz = 146; break;
		case 31: MHz = 147; break;
		default: MHz = 0; break;
		}

	freqVal = MHz    * 1e6
			+ kHzX10 * 1e4
			+ HzX100 * 1e2
			+ Hzx10  * 10
			+ Hz;

#if 0
used for print out
	MHz = freqVal / 1e6;
	kHz = (freqVal - MHz * 1e6) / 1e3;
	Hz = freqVal - MHz * 1e6 - kHz * 1e3;
#endif

	return freqVal;
} */


/*
static int bandIndexToFreq(int bandNum) {

	switch (bandNum) {
	case 0: return 160;
	case 1: return 80;
	case 2: return 60;
	case 3: return 40;
	case 4: return 30;
	case 5: return 20;
	case 6: return 17;
	case 7: return 15;
	case 8: return 12;
	case 9: return 10;
	case 10: return 6;
	default: return -1;
	}
}
*/


static void printRaw(void) {

/*	if (0) {
		printf("Response:\n\n");
		printf("%4s %2s               [addr checksum]\n",
			fmem.addr, fmem.bytesRead);
		printf("%10s %10s [vfo A B]\n", fmem.vfoAfreq, fmem.vfoBfreq);
		printf("%c %c                   [mode B A]\n",
			fmem.vfoAmode, fmem.vfoBmode);
		printf("%c %c %c                 [dataMode Aflags Bflags]\n",
			fmem.dataMode, fmem.vfoFlags1, fmem.vfoFlags2);
		printf("%c                     [xvtrFlags]\n", fmem.xvtrFlags);
		printf("%c                     [unknown]\n", fmem.bandNum);
		printf("%2s                    [pl tone]\n", fmem.plHex);
		printf("%2s                    [repeater offset]\n", fmem.rptrOffset);
		printf("%c                     [unknown]\n", fmem.flags4);
		printf("%c                     [FM info]\n", fmem.fmInfo);
		printf("%8s\n", fmem.flags2);
		printf("%18s\n", fmem.empty1);
		spaceHexString(fmem.label, 2);
		printf("        [label]\n");
		printf("%54s\n", fmem.empty2);
		printf("%2s                    [checksum]\n\n", fmem.checksumResp);
		} else { */
		spaceHexString(erResponse+2, 10);
		printf("\n");
/*	} */

}



/*  (removed from k3comms.c) */


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

#if 0
static void abSwap(int fd) {
	char cmd[7];

	sprintf(cmd, "SWT11;");
	write(fd, cmd, strlen(cmd));
	usleep(100e3);
}


#endif
