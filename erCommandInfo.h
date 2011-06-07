
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

	char * (*getLabel)();
	char * (*getVfoAMode)();
	char * (*getVfoBMode)();
	float (*getPlTone)();
	int (*getAnt)();
	int (*getRepeaterOffset)();
	int (*getVfoAFreq)();
	int (*getVfoBFreq)();
	int (*isAttOn)();
	int (*isModeFm)();
	int (*isNrOn)();
	int (*isPreampOn)();
	int (*isSplitOn)();
	int (*isRxAntOn)();
	void (*setErResponse)(char *);
	void (*printBrief)();
	void (*printRaw)();
	void (*printVerbose)();

} k3FreqMemInfo;

#include "erCommand.h"
