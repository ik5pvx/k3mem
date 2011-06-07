#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "decode.h"

#define BASE_ADDR    0x0C00
#define FREQ_MEM_LEN 0x40

memIndexToAddr(int index, char * cmd) {

	/* Create ER command leaving room for checksum. */
	int addr =  BASE_ADDR + (index * FREQ_MEM_LEN);
	sprintf(cmd, "ER%04X%02X__;", addr, FREQ_MEM_LEN);

	/* Fill in the underscores in above string with checksum. */
	char * cs = malloc(3);
	checksum(cmd, 2, 2+6, cs);
	strncpy(cmd+2+6, cs, 2);
	free(cs);
}

parseERcmd(char * erCmd, k3FreqMem * fmem) {

	sscanf(erCmd,
		"ER%4s%2s%10s%10s%c%c%c%c%c%c%c%c%c%c%c%c%2s%2s%c%c%8s%18s%10s%54s%2s;",
		fmem->addr,
		fmem->bytesRead,
		fmem->vfoAfreq,
		fmem->vfoBfreq,

		&fmem->vfoBmode,
		&fmem->vfoAmode,
		&fmem->dataMode,
		&fmem->vfoFlags1,
		&fmem->vfoFlags2, // Bit 0 seems to be 0/1==lsb/usb
		&fmem->vfoFlags3,
		&fmem->vfoFlags4,
		&fmem->vfoFlags5,
		&fmem->vfoFlags6,
		&fmem->vfoFlags7,
		&fmem->xvtrFlags,
		&fmem->bandNum,
		fmem->plHex,
		fmem->rptrOffset,
		&fmem->flags4,
		&fmem->fmInfo,
		fmem->flags2,

		fmem->empty1,
		fmem->label,
		fmem->empty2,
		fmem->checksumResp
	);
}

decodeBrief(k3FreqMem * fmem) {

	if (!strcmp(fmem->vfoAfreq, "FFFFFFFFFF")
		&& !strcmp(fmem->vfoBfreq, "FFFFFFFFFF")) {
		/* Empty memory location.  Don't try to decode. */
		printf("\n");
		return;
	}

	char * respA = malloc(256);
	char * respB = malloc(256);
	char * lbl = malloc(6);

	printFreq(fmem->vfoAfreq, fmem->xvtrFlags, respA);
	printFreq(fmem->vfoBfreq, fmem->xvtrFlags, respB);
	printLabel(fmem->label, lbl);

	if (strcmp(lbl, "     "))
		printf("%s   ", lbl);
	else
		printf("        ");
	printf("%14s / %-3s    %14s / %-3s", respA,
		mode(fmem->vfoAmode, fmem->vfoFlags2, fmem->vfoFlags7,
			fmem->fmInfo, fmem->dataMode),
		respB, mode(fmem->vfoBmode, fmem->vfoFlags2, fmem->vfoFlags7,
			fmem->fmInfo, fmem->dataMode));
	float pl = plTone(fmem->plHex);
	if (modeFm(fmem->vfoAmode) && pl > 0)
		printf("    PL %.1f Hz", pl);
	printf("\n");

	free(respA);
	free(respB);
	free(lbl);
}

decodeRaw(k3FreqMem * fmem) {

	printf("Response:\n\n");
	printf("%4s %2s               [addr checksum]\n",
		fmem->addr, fmem->bytesRead);
	printf("%10s %10s [vfo A B]\n", fmem->vfoAfreq, fmem->vfoBfreq);
	printf("%c %c                   [mode B A]\n",
		fmem->vfoAmode, fmem->vfoBmode);
	printf("%c %c %c                 [dataMode Aflags Bflags]\n",
		fmem->dataMode, fmem->vfoFlags1, fmem->vfoFlags2);
	printf("%c                     [xvtrFlags]\n", fmem->xvtrFlags);
	printf("%c                     [unknown]\n", fmem->bandNum);
	printf("%2s                    [pl tone]\n", fmem->plHex);
	printf("%2s                    [repeater offset]\n", fmem->rptrOffset);
	printf("%c                     [unknown]\n", fmem->flags4);
	printf("%c                     [FM info]\n", fmem->fmInfo);
	printf("%8s\n", fmem->flags2);
	printf("%18s\n", fmem->empty1);
	spaceHexString(fmem->label, 2);
	printf("        [label]\n");
	printf("%54s\n", fmem->empty2);
	printf("%2s                    [checksum]\n\n", fmem->checksumResp);

}

decodeVerbose(k3FreqMem * fmem) {

	/* sscanf(response, "ER%136s;", wtf); */

	if (!strcmp(fmem->vfoAfreq, "FFFFFFFFFF")
		&& !strcmp(fmem->vfoBfreq, "FFFFFFFFFF")) {
		/* Empty memory location.  Don't try to decode. */
		printf("\n");
		return;
	}

	char * respA = malloc(256);
	char * respB = malloc(256);
	char * lbl = malloc(6);

	printFreq(fmem->vfoAfreq, fmem->xvtrFlags, respA);
	printFreq(fmem->vfoBfreq, fmem->xvtrFlags, respB);
	printLabel(fmem->label, lbl);

	printf("Address:    0x%s\n", fmem->addr);
	printf("Bytes read: 0x%s\n", fmem->bytesRead);
	if (strcmp(lbl, "     "))
		printf("Label: %s\n", lbl);
	printf("VFO A: %s / %s\n", respA, mode(fmem->vfoAmode, fmem->vfoFlags2,
		fmem->vfoFlags7, fmem->fmInfo, fmem->dataMode));
	printf("VFO B: %s / %s\n", respB, mode(fmem->vfoBmode, fmem->vfoFlags2,
		fmem->vfoFlags7, fmem->fmInfo, fmem->dataMode));
	float pl = plTone(fmem->plHex);
	if (modeFm(fmem->vfoAmode) && pl > 0) {
		printf("PL tone:         %.1f Hz\n", pl);
		printf("Repeater offset: %i Hz\n", repeaterOffset(fmem->rptrOffset));
	}
	printf("Split:      %s\n", splitOn(fmem->vfoFlags4) ? "on" : "off");
	printf("Antenna %c:  on\n", ant2(fmem->vfoFlags1) ? '2' : '1');
	printf("Rx antenna: %s\n", rxAntOn(fmem->vfoFlags3) ? "on" : "off");
	printf("Attenuator: %s\n", attOn(fmem->vfoFlags3) ? "on" : "off");
	printf("Preamp:     %s\n", preampOn(fmem->vfoFlags2) ? "on" : "off");
	printf("Noise blanker:   %s\n", nbOn(fmem->vfoFlags1) ? "on" : "off");
	printf("Noise reduction: %s\n", nrOn(fmem->vfoFlags5) ? "on" : "off");
	/*	printf("WTF: %s\n",wtf);*/
	/*
	printf ("Checksum: "); 
	checksum(wtf);
	*/

	free(respA);
	free(respB);
	free(lbl);
}

printFreq(char fStr[], char xvtrFlags, char *freq) {

	int MHz, kHz, kHzX10, HzX100, Hzx10, Hz;
	int freqVal;

	if (!strcmp(fStr, "FFFFFFFFFF")) {
	  sprintf(freq," ");
		return;
	}
	sscanf(fStr, "%2x%2x%2x%2x%2x", &MHz, &kHzX10, &HzX100, &Hzx10, &Hz);

	if (xvtrFlags & 1) /* 2m only? */
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

	MHz = freqVal / 1e6;
	kHz = (freqVal - MHz * 1e6) / 1e3;
	Hz = freqVal - MHz * 1e6 - kHz * 1e3;

	sprintf(freq, "%d %03d %03d Hz", MHz, kHz, Hz);
}

printLabel(char label[], char * asciiLabel) {

	unsigned int c1, c2, c3, c4, c5;

	if (!strcmp(label, "FFFFFFFFFF")) {
		sprintf(asciiLabel, "     ");
		return;
	}

	sscanf(label, "%2x%2x%2x%2x%2x", &c1, &c2, &c3, &c4, &c5);

	sprintf(asciiLabel, "%c%c%c%c%c",
		decodeChar(c1),
		decodeChar(c2),
		decodeChar(c3),
		decodeChar(c4),
		decodeChar(c5));
}

bandIndexToFreq(int bandNum) {

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

decodeChar(unsigned int c) {
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

int ant1(char state)      { return !ant2(state); }

char * mode(char modeChar, char altChar, char flags7, char fmInfo,
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

repeaterOffset(char * rptrOffset) {

	int hz;

	sscanf(rptrOffset, "%2X", &hz);
	return 20 * hz;
}

modeCw(char state)      { return stox(state) == 0; } 
modeLsb(char state)     { return stox(state) == 1; } 
modeUsb(char state)     { return stox(state) == 2; } 
modeData(char state)    { return stox(state) == 3; } 
modeAm(char state)      { return stox(state) == 4; } 
modeFm(char state)      { return stox(state) == 5; } 

int nbOn(char state)      { return stox(state) & 0x2; } 

/* modeAlt 'state' is vfoAflags1 */
modeAlt(char state)   { return stox(state) & 0x4; } 

int ant2(char state)      { return stox(state) & 0x8; }

int preampOn(char state)  { return stox(state) & 0x8; }

int attOn(char state)     { return stox(state) & 0x1; }
int rxAntOn(char state)   { return stox(state) & 0x8; }

int splitOn(char state)   { return stox(state) & 0x1; } 

int nrOn(char state)      { return stox(state) & 0x4; } 

float plTone(char * plHex) {

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

unsigned char stox(unsigned char byte) {

	int c;

	sscanf(&byte, "%x", &c);
	/*
	printf("converted %c to %d\n", byte, c);
	*/
	return c;
}

/*
   1. Convert every two ascii chars into hex int.
   2. Sum these ints.
   3. Subtract from 0x100 (256) to create checksum.
   4. Convert checksum to ascii and return.
 */
checksum(char asciiHex[], int from, int to, char * checksumAscii) {

	char *t;
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
}

checksumOrig(char wtf[]) {
  unsigned int b=0;
  int i;
  char *t;
  unsigned int c=0;
  for (i=0; i<136; i+=2) {
    t=strndup(wtf+i,2);
    sscanf(t,"%2x",&b);
    c = (c+b)%0x80;
    printf ("%d %d %s %d\n",i,b,t,c);

  }
}

spaceHexString(char * hexStr, int interval) {

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
