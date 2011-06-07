#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "decode.h"

main(int argc, char *argv[]) {

	char c, *cmd, *response;
	int gotAddress = 0,
		gotBrief = 0,
	    gotMemChoice = 0,
	    gotMemIndex = 0,
		gotRaw = 0,
		gotResponse = 0,
		i, j,
		index;
	k3FreqMem fmem;

	while ((c = getopt(argc, argv, "a:bimrs:")) != EOF) {
		switch (c) {
		case 'a':
			gotAddress = 1;
			cmd = strdup(optarg);
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
			gotResponse = 1;
			parseERcmd(optarg, &fmem);
			decodeVerbose(&fmem);
			exit(0);
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	if (gotAddress) {
		int cnt;
		int fd = openPort();
		configurePort(fd);
		sscanf(cmd, "%*c%2X;", &cnt);
		printf("cmd is '%s', cnt = %d\n", cmd, cnt);
		response = k3Command(fd, cmd, 10, cnt);
		spaceHexString(response, 10);
		close(fd);
		exit(0); /* More than 1 makes no sense. */
	}

	for (i = optind; i < argc; i++) {

		char *curArg;
		curArg = argv[optind++];
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
				int fd = openPort();
				configurePort(fd);
				setMemChannel(fd, index);
				close(fd);
				exit(0); /* More than 1 makes no sense. */
			}

			if (gotBrief) {
				int i;
				int fd = openPort();
				configurePort(fd);
				cmd = malloc(9);
				memIndexToAddr(index, cmd);
				printf("%3d: ", index);
				/* Send 'cmd' to K3, store response in 'response'. */
				for (i = 0; i < 3; i++) {
					response = k3Command(fd, cmd, 10, 139);
					if (strlen(response) == 139) {
						parseERcmd(response, &fmem);
						decodeBrief(&fmem);
						break;
					}
				}
				free(cmd);
				close(fd);
			}

			if (gotRaw) {
				int i;
				int fd = openPort();
				configurePort(fd);
				cmd = malloc(9);
				memIndexToAddr(index, cmd);
				printf("%3d: ", index);
				/* Send 'cmd' to K3, store response in 'response'. */
				for (i = 0; i < 3; i++) {
					response = k3Command(fd, cmd, 10, 139);
					if (strlen(response) == 139) {
						if (0) {
							/* Some info printed */
							parseERcmd(response, &fmem);
							decodeRaw(&fmem);
						} else {
							/* No extra info printed */
							spaceHexString(response+2, 10);
							printf("\n");
						}
						break;
					}
				}
				free(cmd);
				close(fd);
			}

			if (gotMemIndex) {
				int i;
				int fd = openPort();
				configurePort(fd);
				cmd = malloc(9);
				memIndexToAddr(index, cmd);
				printf("\nMemory channel %d\n", index);
				/* Send 'cmd' to K3, store response in 'response'. */
				for (i = 0; i < 3; i++) {
					response = k3Command(fd, cmd, 10, 139);
					if (strlen(response) == 139) {
						parseERcmd(response, &fmem);
						decodeVerbose(&fmem);
						break;
					}
				}
				free(cmd);
				close(fd);
			}

			if (gotResponse) {
				if (strlen(response) != 139) {
					printf("Command response not 139 chars long.\n");
					printf("Check that you supplied a valid response.\n");
					exit(1);
				}
				parseERcmd(response, &fmem);
				decodeVerbose(&fmem);
			}

		}
	}

}

usage(char * progName) {
	printf("Usage: %s [-i K3memIndex] [-r ERresponse]\n", progName);
	exit(1);
}
