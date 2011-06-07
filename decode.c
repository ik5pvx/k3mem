#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "erCommand.h"

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

decodeBrief(k3FreqMemInfo * fmem) {

	if (fmem-getVfoAFreq != -1 && fmem->getVfoBFreq != -1) {
		/* Empty memory location. */
		printf("\n");
		return;
	}

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

