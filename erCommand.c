/* ----------------------------------------------------------------------------
 *
 * erCommand.c
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
#include <stdlib.h>

#include "erCommand.h"

char * erResponse;
static k3FreqMem fmem;

/*
 *                   P u b l i c   M e t h o d s
 */

/*
 * checksum
 *
   1. Convert every two ascii chars into hex int.
   2. Sum these ints.
   3. Subtract from 0x100 (256) to create checksum.
   4. Convert checksum to ascii and return.
 */
int checksum(char asciiHex[], int from, int to, char * checksumAscii) {

	int i, sum;
	unsigned int b;

	sum = 0;
	for (i = from; i < to; i += 2) {
		sscanf(asciiHex + i, "%2x%*s", &b);
		sum += b;
	}
	sum = 0x100 - sum;
	sum &= 0xFF; /* same as: sum = sum % 0x100 */

	sprintf(checksumAscii, "%2X", sum);
	return sum;
}

/*
 * spaceHexString
 *
 * Print out a string with spaces at specified intervals.  Used in the hope
 * of making certain outputs, like long strings of characters, more readable.
 */

void spaceHexString(char * hexStr, int interval) {

	int col, i, numColumns;

	numColumns = 160;
	col = 1; /* Column currently being printed. */
	for (i = 0; i < strlen(hexStr); i++) {
		if (i > 0 && i % interval == 0)
			printf(" ");
		if (numColumns - col < interval) {
			/* Break line length at last space before column numColumns. */
			printf("\n");
			col = 0;
		}
		col++;
		printf("%c", hexStr[i]);
	}
}

/*
 *                   P r i v a t e   M e t h o d s
 */

/*
 * parseResponse
 *
 * This method is takes the response from an ER (EEPROM Read) command and
 * parses it.  The parsed fields are stored in a private struct.  Only after
 * that struct is initialized can the rest of the methods be called.
 */

static void parseResponse(char * erCmd) {

	if (erResponse != NULL)
		free(erResponse);

	if (strlen(erCmd) != 139) {
		printf("ER response should be 139 characters long, but only got %d.\n",
			   (int)strlen(erCmd));
		return;
	}

	erResponse = strdup(erCmd);

	sscanf(erCmd,
/*		"ER%4s%2s%10c%10c%c%c%c%c%c%c%c%c%c%c%c%c%2s%2s%c%c%8s%18s%10s%54s%2s;",*/
		   "ER%4s%2s%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%c%c%c%c%c%c%c%c%c%c%c%c%2s%2s%c%c%8s%18s%10s%54s%2s;",
		   fmem.addr,
		   fmem.bytesRead,
		   &fmem.vfoAfreq.MHz,&fmem.vfoAfreq.kHz10,&fmem.vfoAfreq.hHz,&fmem.vfoAfreq.daHz,&fmem.vfoAfreq.Hz,
		   &fmem.vfoBfreq.MHz,&fmem.vfoBfreq.kHz10,&fmem.vfoBfreq.hHz,&fmem.vfoBfreq.daHz,&fmem.vfoBfreq.Hz,

		&fmem.vfoBmode,
		&fmem.vfoAmode,
		&fmem.dataMode,
		&fmem.vfoFlags1,
		&fmem.vfoFlags2, // Bit 0 seems to be 0/1==lsb/usb
		&fmem.vfoFlags3,
		&fmem.vfoFlags4,
		&fmem.vfoFlags5,
		&fmem.vfoFlags6,
		&fmem.vfoFlags7,
		&fmem.xvtrFlags,
		&fmem.bandNum,
		fmem.plHex,
		fmem.rptrOffset,
		&fmem.flags4,
		&fmem.fmInfo,
		fmem.flags2,

		fmem.empty1,
		fmem.label,
		fmem.empty2,
		fmem.checksumResp
	);
}

/*
 * freqA
 *
 * Return the VFO A frequency in Hz.
 */

static int freqA(void) {
	return calcFreq(fmem.vfoAfreq, fmem.xvtrFlags);
}

/*
 * freqB
 *
 * Return the VFO B frequency in Hz.
 */

static int freqB(void) {
	return calcFreq(fmem.vfoBfreq, fmem.xvtrFlags);
}

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

int calcFreq(k3VfoFreq f, char xvtrFlags) {
	int freqVal;

	if ( f.MHz == 0xFF )
		return -1;
	
	freqVal = f.MHz * 1e6
		+ f.kHz10   * 1e4
		+ f.hHz     * 1e2
		+ f.daHz    * 10
		+ f.Hz;

	if (xvtrFlags & 1)
		printf ("FOO\n"); /* FIXME: get transverter base frequency and add it */

	return freqVal;
}

/*
 * decodeChar
 *
 * Convert a K3 encoded character to ascii.  The K3 encodes alphanumerics and
 * a small number of additional characters.
 */

static char decodeChar(unsigned int c) {
	char cvt;

	if (c >= 0x01 && c <= 0x1A)
		cvt = c + 0x40; // Convert K3 letter to ascii
	else if (c >= 0x1B && c <= 0x24)
		cvt = c + 0x15; // Convert K3 digit to ascii
	else
		switch (c) {    // Convert K3 non-alphanumeric to ascii
		case 0x00: cvt = ' '; break;
		case 0x25: cvt = '*'; break;
		case 0x26: cvt = '+'; break;
		case 0x27: cvt = '/'; break;
		case 0x28: cvt = '@'; break;
		case 0x29: cvt = '_'; break;
		default:   cvt = '?'; break;
		}

	return cvt;
}

/*
 * getLabel
 *
 * Given an ER substring containing a frequency memory text label, decode
 * that label from K3 encoding to ascii.
 */

static char * getLabel(void) {

	char * asciiLabel;
	unsigned int c1, c2, c3, c4, c5;

	asciiLabel = malloc(6);

	if (!strcmp(fmem.label, "FFFFFFFFFF")) {
		sprintf(asciiLabel, "     ");
	} else {
		sscanf(fmem.label, "%2x%2x%2x%2x%2x", &c1, &c2, &c3, &c4, &c5);
		sprintf(asciiLabel, "%c%c%c%c%c",
			decodeChar(c1),
			decodeChar(c2),
			decodeChar(c3),
			decodeChar(c4),
			decodeChar(c5));
	}

	return asciiLabel;
}

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

static unsigned char stox(const char byte) {

	int c;

	sscanf(&byte, "%x", &c);
	/*
	printf("converted %c to %d\n", byte, c);
	*/
	return c;
}

/*
 * mode
 *
 * Given information from an ER response, this method will determine the
 * operational mode - CW, LSB, etc., - associated with a stored frequency.
 */

static const char * mode(char modeChar, char altChar, char flags7, char fmInfo,
	char dataMode) {

	int modeByte, revCw, revMode;

	revCw = stox(altChar) & 0x4; // See if cw/data ALT bit is set.
	revMode = stox(altChar) & 0x1; // See if non cw/data ALT bit is set.
	flags7 = stox(flags7);
	dataMode = stox(dataMode);
	modeByte = stox(modeChar);
	fmInfo = stox(fmInfo);
	switch(modeByte) {
	case 0: return revCw ? "CW REV" : "CW";
	case 1: return "LSB";
	case 2: return "USB";
	case 3:
		if (revCw)
			switch (dataMode) {
			case 0: return "DATA REV, data A";
			case 2: return "DATA REV, afsk A";
			case 4: return "DATA REV, fsk D";
			case 6: return "DATA REV, psk D";
			default: return "DATA REV, ???";
			}
		else
			switch (dataMode) {
			case 0: return "DATA, data A";
			case 2: return "DATA, afsk A";
			case 4: return "DATA, fsk D";
			case 6: return "DATA, psk D";
			default: return "DATA, ???";
			}
	case 4: return flags7 & 0x8 ? "AM-S" : "AM";
	case 5: 
		if (fmInfo == 0x04)
			return "FM";
		else if (fmInfo == 0x05)
			return "FM+";
		else if (fmInfo == 0x06)
			return "FM-";
		else {
			return "FM?";
		}
	default:
		return "?";
	}
}

/*
 * modeA
 *
 * Return a string describing the VFO A mode.
 */

static const char * modeA(void) {
	return mode(fmem.vfoAmode, fmem.vfoFlags2,
		fmem.vfoFlags7, fmem.fmInfo, fmem.dataMode);
}

/*
 * modeB
 *
 * Return a string describing the VFO B mode.
 */

static const char * modeB(void) {
	return mode(fmem.vfoBmode, fmem.vfoFlags2,
		fmem.vfoFlags7, fmem.fmInfo, fmem.dataMode);
}

/*
 * repeaterOffset
 *
 * Returns the offset in Hz as encoded in a K3 frequency memory.
 */

static int repeaterOffsetStr(const char * rptrOffset) {

	int hz;

	sscanf(rptrOffset, "%2X", &hz);
	return 20 * hz;
}

static int repeaterOffset(void) {
	return repeaterOffsetStr(fmem.rptrOffset);
}

/*
static int isModeCw(void)   { return stox(fmem.vfoAmode) == 0; } 
static int isModeLsb(void)  { return stox(fmem.vfoAmode) == 1; } 
static int isModeUsb(void)  { return stox(fmem.vfoAmode) == 2; } 
static int isModeData(void) { return stox(fmem.vfoAmode) == 3; } 
static int isModeAm(void)   { return stox(fmem.vfoAmode) == 4; } 
*/
static int isModeFm(void)   { return stox(fmem.vfoAmode) == 5; }
/*
static int modeAlt(void)    { return stox(fmem.vfoFlags1) & 0x4; } 
*/
static int nbOn(void)       { return stox(fmem.vfoFlags2) & 0x2; } 
static int ant(void)        { return (stox(fmem.vfoFlags2) & 0x8) ? 2 : 1; }

static int preampOn(void)   { return stox(fmem.vfoFlags3) & 0x8; }

static int attOn(void)      { return stox(fmem.vfoFlags4) & 0x1; }
static int rxAntOn(void)    { return stox(fmem.vfoFlags4) & 0x8; }

static int splitOn(void)    { return stox(fmem.vfoFlags5) & 0x1; } 

static int nrOn(void)       { return stox(fmem.vfoFlags6) & 0x4; } 

static float plToneStr(const char * plHex) {

	float hz;
	int hexHz;

	sscanf(plHex, "%2X", &hexHz);

	switch (hexHz) {
	case 0x01: hz = 67.0; break;
	case 0x02: hz = 69.3; break;
	case 0x03: hz = 71.9; break;
	case 0x04: hz = 74.4; break;
	case 0x05: hz = 77.0; break;
	case 0x06: hz = 79.7; break;
	case 0x07: hz = 82.5; break;
	case 0x08: hz = 85.4; break;
	case 0x09: hz = 88.5; break;
	case 0x0a: hz = 91.5; break;
	case 0x0b: hz = 94.8; break;
	case 0x0c: hz = 97.4; break;
	case 0x0d: hz = 100.0; break;
	case 0x0e: hz = 103.5; break;
	case 0x0f: hz = 107.2; break;
	case 0x10: hz = 110.9; break;
	case 0x11: hz = 114.8; break;
	case 0x12: hz = 118.8; break;
	case 0x13: hz = 123.0; break;
	case 0x14: hz = 127.3; break;
	case 0x15: hz = 131.8; break;
	case 0x16: hz = 136.5; break;
	case 0x17: hz = 141.3; break;
	case 0x18: hz = 146.2; break;
	case 0x19: hz = 151.4; break;
	case 0x1a: hz = 156.7; break;
	case 0x1b: hz = 159.8; break;
	case 0x1c: hz = 162.2; break;
	case 0x1d: hz = 165.5; break;
	case 0x1e: hz = 167.9; break;
	case 0x1f: hz = 171.3; break;
	case 0x20: hz = 173.8; break;
	case 0x21: hz = 177.3; break;
	case 0x22: hz = 179.9; break;
	case 0x23: hz = 183.5; break;
	case 0x24: hz = 186.2; break;
	case 0x25: hz = 189.9; break;
	case 0x26: hz = 192.8; break;
	case 0x27: hz = 196.6; break;
	case 0x28: hz = 199.5; break;
	case 0x29: hz = 203.5; break;
	case 0x2a: hz = 206.5; break;
	case 0x2b: hz = 210.7; break;
	case 0x2c: hz = 218.1; break;
	case 0x2d: hz = 225.7; break;
	case 0x2e: hz = 229.1; break;
	case 0x2f: hz = 233.6; break;
	case 0x30: hz = 241.8; break;
	case 0x31: hz = 250.3; break;
	case 0x32: hz = 254.1; break;
	case 0x33: hz = 1750.0; break;
	default: hz = 0; break;
	}

	return hz;
}

static float plTone(void)   { return plToneStr(fmem.plHex); }

static void printBrief(void) {

	char *lbl;
	int fa, fb;

	fa = freqA();
	fb = freqB();

	if (fa == -1)
		/*printf("- / -   ");*/
		printf("(empty)\n");
	else {
		float pl;

		lbl = getLabel();
		printf("%6s:", lbl);
		free(lbl);
		printf("%10d%10s", fa, modeA());
		if (fb == -1)
			printf("%23s","");
		else
			printf(" / %10d%10s", fb, modeB());
		pl = plTone();
		if (isModeFm() && pl > 0)
			printf("    PL %.1f Hz", pl);
		printf(".\n"); /* FIXME: try some memories with the pl tone and 
						  get the spacing right */
	}
}

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

static void printVerbose(void) {
	char *lbl;
	float pl;

	/* sscanf(response, "ER%136s;", wtf); */

	int fa = freqA();
	int fb = freqB();

	if (fa == -1) {
		/* Empty memory location. */
		printf("\n");
		return;
	}

	printf("Address:    0x%s\n", fmem.addr);
	printf("Bytes read: 0x%s\n", fmem.bytesRead);
	lbl = getLabel();
	if (strcmp(lbl, "     "))
		printf("Label: %s\n", lbl);
	free(lbl);
/*	if (fa == -1)
		printf("VFO A: \n");
		else*/
		printf("VFO A: %d / %s\n", freqA(), modeA());
	if (fb == -1)
		printf("VFO B: \n");
	else
		printf("VFO B: %d / %s\n", freqB(), modeB());
	pl = plTone();
	if (isModeFm() && pl > 0) {
		printf("PL tone:         %.1f Hz\n", pl);
		printf("Repeater offset: %i Hz\n", repeaterOffset());
	}
	printf("Split:      %s\n", splitOn() ? "on" : "off");
	printf("Antenna %d:  on\n", ant());
	printf("Rx antenna: %s\n", rxAntOn() ? "on" : "off");
	printf("Attenuator: %s\n", attOn() ? "on" : "off");
	printf("Preamp:     %s\n", preampOn() ? "on" : "off");
	printf("Noise blanker:   %s\n", nbOn() ? "on" : "off");
	printf("Noise reduction: %s\n", nrOn() ? "on" : "off");
	/*	printf("WTF: %s\n",wtf);*/
	/*
	printf ("Checksum: "); 
	checksum(wtf);
	*/
}

/*
 *   newk3FreqMemInfo() -  k3FreqMemInfo constructor
 *
 *   Create class and provide public methods.  Nearly all methods
 *   (subroutines) in this file are private (static) and can only be accessed
 *   by the function pointers initialized in the constructor.
 *
 *   The associated header file shows function return values and parameters.
 */

k3FreqMemInfo * newK3FreqMemInfo() {

	k3FreqMemInfo * m;

	m = (k3FreqMemInfo *) malloc(sizeof(k3FreqMemInfo));
	m->getAnt = &ant;
	m->getLabel = &getLabel;
	m->getPlTone = &plTone;
	m->getVfoAFreq = &freqA;
	m->getVfoAMode = &modeA;
	m->getVfoBFreq = &freqB;
	m->getVfoBMode = &modeB;
	m->isAttOn = &attOn;
	m->isModeFm = &isModeFm;
	m->isNrOn = &nrOn;
	m->isPreampOn = &preampOn;
	m->isRxAntOn = &rxAntOn;
	m->isSplitOn = &splitOn;
	m->printBrief = &printBrief;
	m->printRaw = &printRaw;
	m->printVerbose = &printVerbose;
	m->setErResponse = &parseResponse;

	return m;
}
