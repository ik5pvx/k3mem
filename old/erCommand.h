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
static void parseResponse(char *, k3FreqMem *);
static void decodeBrief(k3FreqMem *);
static void decodeRaw(k3FreqMem *);
static void decodeVerbose(k3FreqMem *);
static int calcFreq(char[], char);
static char * printLabel(char[], char *);
static int bandIndexToFreq(int);
static char decodeChar(unsigned int);
static int ant1(char);
static char * mode(char, char, char, char, char);
static int repeaterOffset(char *);
static int modeCw(char);
static int modeLsb(char);
static int modeUsb(char);
static int modeData(char);
static int modeAm(char);
static int modeFm(char);
static int nbOn(char);
static int modeAlt(char);
static int ant2(char);
static int preampOn(char);
static int attOn(char);
static int rxAntOn(char);
static int splitOn(char);
static int nrOn(char);
static float plTone(char *);
static unsigned char stox(unsigned char);
static int checksum(char[], int, int, char *);
static int checksumOrig(char[]);

