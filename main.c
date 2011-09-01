/* ----------------------------------------------------------------------------
 *
 * main.c
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
#include <getopt.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>

#include "erCommand.h"
#include "k3comms.h"

static void usage(char * progName) {
	printf("Usage:\n%s [-s speed] [-d device] [-(i|r|b) K3memIndex] : read memories from radio \n", progName);
	printf("%s [-a K3memIndex] : translate memory channel number to address \n", progName);
	printf("%s [-x ERresponse] : translate a raw response \n", progName);
	printf("%s [-m channel]    : set a memory channel on the radio (like pressing MR) \n", progName);
	exit(1);
}

static void memIndexToAddr(int idx, char * cmd) {
	char *cs = NULL;

	/* Create ER command leaving room for checksum. */
	int addr =  BASE_ADDR + (idx * FREQ_MEM_LEN);
	sprintf(cmd, "ER%04X%02X__;", addr, FREQ_MEM_LEN);

	/* Fill in the underscores in above string with checksum. */
	cs = malloc(3);
	checksum(cmd, 2, 2+6, cs);
	strncpy(cmd+2+6, cs, 2);
}

static void retrieveAddress(char * addr,char *device,int speed, int argspeed) {
	char *isK3;
	int cnt, fd;
	char cmdK3[4]="K3;";

	fd = openPort(device);
	configurePort(fd,speed);
	isK3 = k3Command(fd,cmdK3,50,4);

	if ( (strncmp(isK3,"K3",2) == 0) ) {
		char *response = NULL;

		sscanf(addr, "%*c%2X;", &cnt);
		printf("cmd is '%s', cnt = %d\n", addr, cnt);
		response = k3Command(fd, addr, 10, cnt);
		spaceHexString(response, 10);
		free(response);
	} else {
		fprintf(stderr,
				"Whatever is connected on %s at %d baud is not a K3.\nResponse was: %s\n",
				device, argspeed, isK3);
	}
	close(fd);
}

static void ascii2bin (char *a, char *b, int sa) {
	int sb=sa/2;
/*	printf("HHHH %d %d\nGoo:",sa,sb);*/
	int i;
	unsigned int v;
	for (i=0;i<sb;i++) {
		sscanf(a+i*2,"%2x",&v);
/*		printf("%d",v);*/
		b[i]=v;
	}
/*	printf("\n");*/
}

static void BandMemoriesStreamtoStruct (char *response,int idx,int count,k3BandMemory **bandmemory) {
/* not a good idea
	char format[139]="ER%6c";
	int i;
	for (i=0;i<count;i++){
		strcat(format,"%32c");
	}
	strcat(format,"%2c;");
	fprintf(stderr,"%s %d\n",format, idx);
*/	
	int i;
	char asciirecord[33];
	char record[16];
	for (i=0;i<count;i++) {
		sscanf(response+8+i*32,"%32c",asciirecord);
		asciirecord[32]='\0';
		fprintf(stderr,"%s\n",asciirecord);
		bandmemory[idx+i] = (k3BandMemory *)malloc(sizeof(k3BandMemory));
		ascii2bin(asciirecord,record,sizeof(asciirecord));
		memcpy(bandmemory[idx+i],record,16);
	}
}

static void getBandMemories(int fd,k3BandMemory **bandmemory) {
	int i, addr, count;
	char cmd[12];
	char *cs = NULL, *response = NULL;

	/* read 4 bandmemories at a time, including reserved ones 
	   plus the last one (25 bands total) */
	count = 4 * BANDMEMORY_SIZE;
	for (i=0;i<6;i++) { 
		addr = NORMAL_BANDMEMORY_START + i * count;
		sprintf(cmd,"ER%04X%02X__;",addr,count);

		/* Fill in the underscores in above string with checksum. */
		cs = malloc(3);
		checksum(cmd, 2, 2+6, cs);
		strncpy(cmd+2+6, cs, 2);
		free(cs);

		fprintf(stderr,"Read Band Memory: %s\n",cmd);

		response = malloc(139);
		response = k3Command(fd,cmd,50,139);
		response[139] = '\0'; /* FIXME: k3Command should do this! */
		fprintf(stderr,"Response: %s\n",response);
		/* FIXME: verify checksum */
		BandMemoriesStreamtoStruct(response,i*4,4,bandmemory);
		free (response);

	}
	addr = NORMAL_BANDMEMORY_START + BANDMEMORY_SIZE * 24;
	sprintf(cmd,"ER%04X%02X__;",addr,BANDMEMORY_SIZE);
	cs = malloc(3);
	checksum(cmd, 2, 2+6, cs);
	strncpy(cmd+2+6, cs, 2);
	free(cs);
	fprintf(stderr,"Read Band Memory: %s\n",cmd);
	response = malloc(43);
	response = k3Command(fd,cmd,50,43);
	response[43] = '\0'; /* FIXME: k3Command should do this! */
	fprintf(stderr,"Response: %s\n",response);
	/* FIXME: verify checksum */
	BandMemoriesStreamtoStruct(response,i*4,1,bandmemory);
	free (response);
}

static void getTransverterState(int fd) {
}

static void decodeBandMemories (k3BandMemory *bandmemory) {
	int f;
	f=calcFreq(bandmemory->vfoAfreq,bandmemory->x4);
	printf ("Gaaah!\n%s\n%d\n",(char *)bandmemory,f); /* this is nuts */
	
}

int main(int argc, char *argv[]) {
	char c, cmd[9];
	char cmdK3[4]="K3;";
	char *response = NULL, *isK3 = NULL;
	char device[PATH_MAX+1]; /* maximum path length supported by the OS, from limits.h, plus \0 */

	int gotBrief = 0,         /* -b */
	    gotMemChoice = 0,     /* -m */
	    gotMemIndex = 0,      /* -i */
	    gotRaw = 0;           /* -r */
	int i, idx;
	int longval;
	struct option long_options[] = {
		{"address",required_argument,0,'a'},
		{"brief",optional_argument,0,'b'},
		{"device",required_argument,0,'d'},
		{"index",optional_argument,0,'i'},
		{"memset",optional_argument,0,'m'},
		{"raw",optional_argument,0,'r'},
		{"speed",required_argument,0,'s'},
		{"version",no_argument,&longval,'v'},
		{"extract",required_argument,0,'x'},
		{0,0,0,0}
	};
	int option_idx = 0;
	int speed=B38400;
	int argspeed=0;
	int fd;
	k3FreqMemInfo *memInfo;

	strncpy(device, "/dev/ttyS0", PATH_MAX);

	memInfo = newK3FreqMemInfo();

	if (argc <= 1) usage(argv[0]);
	while (( c = getopt_long(
				 argc, argv, "a:bd:imrs:vx:", long_options, &option_idx)
			   ) != -1) {
		switch (c) {
		case 'a': /* translate mem channel to memory address */
			retrieveAddress(strdup(optarg),device,speed,argspeed);
			exit(0);
			break;
		case 'b': /* brief listing */
			gotBrief = 1;
			break;
		case 'd': /* serial device specified */
			strncpy(device,optarg,PATH_MAX);
			break;
		case 'i': /* verbose listing */
			gotMemIndex = 1;
			break;
		case 'm': /* select a memory channel */
			gotMemChoice = 1;
			break;
		case 'r': /* raw listing */
			gotRaw = 1;
			break;
		case 's': /* serial speed specified */
			argspeed = atoi(optarg);
			switch (argspeed) {
			case 4800:
				speed=B4800;
				break;
			case 9600:
				speed=B9600;
				break;
			case 19200:
				speed=B19200;
				break;
			case 38400:
				speed=B38400;
				break;
			default: 
				fprintf(stderr,"Unsupported speed %s:\nK3 only allows 4800, 9600, 19200 or 38400\n",optarg);
			exit (1);
			}
			break;
		case 'x': /* translate a raw response */
			printf("%s\n",optarg);
			memInfo->setErResponse(optarg);
			memInfo->printVerbose();
			exit(0);
			break;
		case 0:   /* this handles longoptions with no short counterpart */
			switch (longval) {
			case 'v': /* version */ /* FIXME: define version somewhere */
				printf("Version undefined! (%c %d)\n", c, option_idx);
				break;
				/* no default necessary here */
			}
			break;
		default:
			printf ("option %s", long_options[option_idx].name);
			if (optarg)
				printf (" with arg %s", optarg);
			printf ("\n");
			usage(argv[0]);
			break;
		}
	}

	fd = openPort(device);
	configurePort(fd,speed);

	isK3 = k3Command(fd,cmdK3,50,4);

	if ( (strncmp(isK3,"K3",2) == 0) ) { 

		k3BandMemory *bandmemory[25];
		getBandMemories(fd,bandmemory);
		getTransverterState(fd);

		decodeBandMemories(bandmemory[10]);

		for (i = optind; i < argc; i++) {

			char *curArg = argv[optind++];
			char *dashIndex = strchr(curArg, '-');
			int loIndex, hiIndex;

			if (dashIndex != NULL)
				sscanf(curArg, "%d-%d", &loIndex, &hiIndex);
			else
				loIndex = hiIndex = atoi(curArg);

			for (idx = loIndex; idx <= hiIndex; idx++) {

				if (idx < 0 || idx > 199)
					usage(argv[0]);

				if (gotMemChoice) {
					setMemChannel(fd, idx);
					exit(0); /* More than 1 makes no sense. */
				}

				memIndexToAddr(idx, cmd); /* Calculate eeprom address of idx. */
				response = k3Command(fd, cmd, 10, 139); /* Send ER cmd. */
				memInfo->setErResponse(response);
				free(response);

				if (gotBrief) {
					printf("%3d: ", idx);
					memInfo->printBrief();
				}
				if (gotRaw) {
					printf("%3d: ", idx);
					memInfo->printRaw();
				}
				if (gotMemIndex) {
					printf("\nMemory channel %d\n", idx);
					memInfo->printVerbose();
				}
			}
		}
	} else {
		fprintf(stderr,
				"Whatever is connected on %s at %d baud is not a K3.\nResponse was: %s\n",
				device, argspeed, isK3);
	}
	close(fd);
	exit(0);
}
