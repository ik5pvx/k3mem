/* ----------------------------------------------------------------------------
 *
 * decode.h
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
float plTone(char *);
unsigned char stox(unsigned char);
char * k3Command(int, char *, int, int);
char * mode(char, char, char, char, char);
int ant2(char);
int attOn(char);
int modeFm(char);
int nbOn(char);
int nrOn(char);
int preampOn(char);
int printFreq(char[], char, char *);
int rxAntOn(char);
int splitOn(char);

typedef struct {

	char addr[5];
	char bytesRead[3];
	char checksumResp[3];
	char empty1[19];
	char empty2[55];
	char bandNum;
	char flags2[9];
	char flags4;
	char fmInfo;
	char label[11];
	char plHex[3];
	char rptrOffset[3];
	char dataMode;
	char xvtrFlags;
	char vfoFlags1;
	char vfoFlags2;
	char vfoFlags3;
	char vfoFlags4;
	char vfoFlags5;
	char vfoFlags6;
	char vfoFlags7;
	char vfoAfreq[11];
	char vfoAmode;
	char vfoBfreq[11];
	char vfoBmode;

} k3FreqMem;

typedef struct {

	char * (*getLabel)();
	char * (*getPlTone)();
	char * (*getVfoAFreq)();
	char * (*getVfoAMode)();
	char * (*getVfoBFreq)();
	char * (*getVfoBMode)();

} k3FreqMemInfo;
