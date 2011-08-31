#ifndef __ER_COMMAND_H__
#define __ER_COMMAND_H__

#include "erCommandInfo.h"

#define BASE_ADDR    0x0C00
#define FREQ_MEM_LEN 0x40

k3FreqMemInfo * newK3FreqMemInfo ( void );
int checksum ( char asciiHex[], int from, int to, char * checksumAscii );
void spaceHexString ( char * hexStr, int interval );
int calcFreq ( k3VfoFreq f, char xvtrFlags );

#endif /* __ER_COMMAND_H__ */
