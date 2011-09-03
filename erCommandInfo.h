/* ----------------------------------------------------------------------------
 *
 * erCommandInfo.h
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
#ifndef __ER_COMMAND_INFO_H__
#define __ER_COMMAND_INFO_H__

/*
 * Structures and constant related to K3 memory access.
 *
 */
typedef struct { 
	unsigned int MHz;
	unsigned int kHz10;
	unsigned int hHz;
	unsigned int daHz;
	unsigned int Hz;
} k3VfoFreq;

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
/*	char vfoAfreq[11];*/
	k3VfoFreq vfoAfreq;
	char vfoAmode;
/*	char vfoBfreq[11];*/
	k3VfoFreq vfoBfreq;
	char vfoBmode;

} k3FreqMem;

typedef struct {

	char * (*getLabel)(void);
	const char * (*getVfoAMode)(void);
	const char * (*getVfoBMode)(void);
	float (*getPlTone)(void);
	int (*getAnt)(void);
	int (*getRepeaterOffset)(void);
	int (*getVfoAFreq)(void);
	int (*getVfoBFreq)(void);
	int (*isAttOn)(void);
	int (*isModeFm)(void);
	int (*isNrOn)(void);
	int (*isPreampOn)(void);
	int (*isSplitOn)(void);
	int (*isRxAntOn)(void);
	void (*setErResponse)(char *);
	void (*printBrief)(void);
	void (*printRaw)(void);
	void (*printLong)(void);

} k3FreqMemInfo;

#define NORMAL_BANDMEMORY_START 0x0100
#define NORMAL_BANDMEMORY_COUNT 11
#define TRANSVERTER_BANDMEMORY_START 0x0200
#define TRANSVERTER_BANDMEMORY_COUNT 9
#define BANDMEMORY_SIZE 0x10
#define TRANSVERTERSTATE_START 0x02A2
#define TRANSVERTERSTATE_COUNT 9
#define TRANSVERTERSTATE_SIZE 0x0A
#define MEMORYCHANNEL_START 0x0C00
#define MEMORYCHANNEL_SIZE 0x40


#define MAX_READ_SIZE 0x40

typedef struct {
	k3VfoFreq vfoAfreq;
	k3VfoFreq vfoBfreq;
	char vfoBmode;
	char vfoAmode;
	char x1; /* mode */
	char x2; /* mode */
	char x3; /* other flags to be decoded */
	char x4; /* other flags to be decoded */
	char x5; /* other flags to be decoded */
} k3BandMemory;

typedef struct {
	char x1; /* some flags that tell if xv is active, 
				if it's internal and if offset is negative */
	char x2; /* IF band (corresponds to band number) */
	char x3; /* base frequency MHz (0-99) */
	char x4; /* base frequency 100xMHz (0-249) */
	char x5; /* offset ? */
	char x6; /* offset ? */
	char x7; /* power in units of 0.1 W (if H set) */
	char x8; /* other flags to be decoded */
	char x9; /* other flags to be decoded */
	char x10;/* other flags to be decoded */
} k3TransverterState;

enum BandIndex {
/* normal bands: 0-10 */
	m160, m80, m60, m40, m30, m20, m17, m15, m12, m10, m6, 
/* reserved bands: 11-15 */
	r11,r12,r13,r14,r15,                                   
/* transverter bands: 16-24  */
	xv1, xv2, xv3, xv4, xv5, xv6, xv7, xv8, xv9
};

#endif /* __ER_COMMAND_INFO_H__ */
