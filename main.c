#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include "erCommandInfo.h"
#include "k3comms.h"

void retrieveAddress(char * addr);

main(int argc, char *argv[]) {

	char *addr, c, cmd[9], *response;
	int gotBrief = 0,         /* -b */
	    gotMemChoice = 0,     /* -m */
	    gotMemIndex = 0,      /* -i */
		gotRaw = 0,           /* -r */
		i, j,
		index;
	k3FreqMemInfo *memInfo;

	memInfo = newK3FreqMemInfo();

	if (argc <= 1) usage(argv[0]);
	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"address",required_argument,0,0},
			{"brief",optional_argument,0,0},
			{"index",optional_argument,0,0},
			{"memset",optional_argument,0,0},
			{"raw",optional_argument,0,0},
			{"show",required_argument,0,0},
		};

		c = getopt_long(argc, argv, "a:bimrs:", long_options, &option_index);
		if (c == -1)
			break;
		printf ("%d %d\n",c,option_index);
		switch (c) {
/*		case '0': */
			/* If this option set a flag, do nothing else now. */
/*			if (long_options[option_index].flag != 0)
				break;
			printf ("option %s", long_options[option_index].name);
			if (optarg)
				printf (" with arg %s", optarg);
			printf ("\n");
			break; */
		case 'a': /* translate mem channel to memory address */
			retrieveAddress(strdup(optarg));
			exit(0);
			break;
		case 'b': /* brief listing */
			gotBrief = 1;
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
		default:
			printf ("option %s", long_options[option_index].name);
			if (optarg)
				printf (" with arg %s", optarg);
			printf ("\n");
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
	printf("Usage:\n%s [-(i|r|b) K3memIndex]\n", progName);
	printf("%s [-a K3memIndex]\n", progName);
	printf("%s [-s ERresponse]\n", progName);
	exit(1);
}
