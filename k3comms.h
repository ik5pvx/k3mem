#ifndef __K3COMMS_H__
#define __K3COMMS_H__

int openPort ( char *device );
void configurePort ( int portFd, int speed );
void setMemChannel ( int fd, int memNum );
char * k3Command ( int fd, char * cmd, int msecSleep, int bytesToRead );

#endif /* __K3COMMS_H__ */
