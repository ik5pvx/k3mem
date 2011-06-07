#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "erCommandInfo.h"
#include "k3comms.h"

void retrieveAddress(char * addr);

main(int argc, char *argv[]) {

	char *addr, c, cmd[9], *response;
	int gotBrief = 0,
	    gotMemChoice = 0,
	    gotMemIndex = 0,
		gotRaw = 0,
		i, j,
		index;
	k3FreqMemInfo *memInfo;

	memInfo = newK3FreqMemInfo();

	while ((c = getopt(argc, argv, "a:bimrs:")) != EOF) {
		switch (c) {
		case 'a':
			retrieveAddress(strdup(optarg));
			exit(0);
			break;
		case 'b':
			gotBrief = 1;
			break;
		case 'i':
			gotMemIndex = 1;
			break;
		case 'm':
			gotMemChoice = 1;
			break;
		case 'r':
			gotRaw = 1;
			break;
		case 's':
			memInfo->setErResponse(optarg);
			memInfo->printVerbose();
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	int fd = openPort();
	configurePort(fd);

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
}

void retrieveAddress(char * addr) {
	int cnt;
	int fd = openPort();
	configurePort(fd);
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
	printf("Usage: %s [-i K3memIndex] [-r ERresponse]\n", progName);
	exit(1);
}
