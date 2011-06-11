#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <termios.h>
#include "erCommandInfo.h"
#include "k3comms.h"

void retrieveAddress(char * addr, char *device, int speed);

main(int argc, char *argv[]) {

	char *addr, c, cmd[9], *response;
	int gotBrief = 0,         /* -b */
	    gotMemChoice = 0,     /* -m */
	    gotMemIndex = 0,      /* -i */
		gotRaw = 0,           /* -r */
/*		gotDevice = 0, */       /* -d */
		i, j, l,
		index;
	int longval;
	struct option long_options[] = {
		{"address",required_argument,0,'a'},
		{"brief",optional_argument,0,'b'},
		{"device",required_argument,0,'d'},
		{"index",optional_argument,0,'i'},
		{"memset",optional_argument,0,'m'},
		{"raw",optional_argument,0,'r'},
		{"show",required_argument,0,'s'},
		{"version",no_argument,&longval,'v'},
		{0,0,0,0}
	};
	int option_index = 0;
	char *device = malloc(256);
	strcpy(device, "/dev/ttyS0");
	int speed=B38400;

	k3FreqMemInfo *memInfo;

	memInfo = newK3FreqMemInfo();

	if (argc <= 1) usage(argv[0]);
	while (( c = getopt_long(
				 argc, argv, "a:bd:imrs:", long_options, &option_index)
			   ) != -1) {
		switch (c) {
		case 'a': /* translate mem channel to memory address */
			retrieveAddress(strdup(optarg),device,speed);
			exit(0);
			break;
		case 'b': /* brief listing */
			gotBrief = 1;
			break;
		case 'd': /* serial device specified */
/*			gotDevice = 1;*/
			printf ("%c %d %s\n",c,option_index,optarg);
			l=strlen(optarg);
			printf ("foo: %d\n",l);
			if (l<255) {
				strncpy(device,optarg,l);
				device[l]='\0';
			} else {
				fprintf(stderr,"Device name too long:\n%s\n",optarg);
				exit (1);
			}
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
		case 's': /* translate a raw response */
			memInfo->setErResponse(optarg);
			memInfo->printVerbose();
			break;
		case 0:   /* this handles longoptions with no short counterpart */
			switch (longval) {
			case 'v': /* version */ /* FIXME: define version somewhere */
				printf("Version undefined! (%c %d)\n", c, option_index);
				break;
				/* no default necessary here */
			}
			break;
		default:
			printf ("option %s", long_options[option_index].name);
			if (optarg)
				printf (" with arg %s", optarg);
			printf ("\n");
			usage(argv[0]);
			break;
		}
	}


	int fd = openPort(device);
	configurePort(fd,speed);

	for (i = optind; i < argc; i++) {

		char *curArg = argv[optind++];
		char *dashIndex = strchr(curArg, '-');
		int loIndex, hiIndex;

		if (dashIndex != NULL)
			sscanf(curArg, "%d-%d", &loIndex, &hiIndex);
		else
			loIndex = hiIndex = atoi(curArg);

		for (index = loIndex; index <= hiIndex; index++) {

			if (index < 0 || index > 199)
				usage(argv[0]);

			if (gotMemChoice) {
				setMemChannel(fd, index);
				exit(0); /* More than 1 makes no sense. */
			}

			memIndexToAddr(index, cmd); /* Calculate eeprom address of index. */
			response = k3Command(fd, cmd, 10, 139); /* Send ER cmd. */
			memInfo->setErResponse(response);
			free(response);

			if (gotBrief) {
				printf("%3d: ", index);
				memInfo->printBrief();
			}
			if (gotRaw) {
				printf("%3d: ", index);
				memInfo->printRaw();
			}
			if (gotMemIndex) {
				printf("\nMemory channel %d\n", index);
				memInfo->printVerbose();
			}
		}
	}
	close(fd);
	free (device);
}

void retrieveAddress(char * addr,char *device,int speed) {
	int cnt;
	int fd = openPort(device);
	configurePort(fd,speed);
	sscanf(addr, "%*c%2X;", &cnt);
	printf("cmd is '%s', cnt = %d\n", addr, cnt);
	char * response = k3Command(fd, addr, 10, cnt);
	spaceHexString(response, 10);
	free(response);
	close(fd);
}

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

usage(char * progName) {
	printf("Usage:\n%s [-(i|r|b) K3memIndex]\n", progName);
	printf("%s [-a K3memIndex]\n", progName);
	printf("%s [-s ERresponse]\n", progName);
	exit(1);
}
