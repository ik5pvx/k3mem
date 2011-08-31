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
