#ifndef _TR
#define _TR
#include <iostream>
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#define TORE "XXX"

#define IROOT "/tmp/ihog/"
#define TOR_DIR "/meta/"

typedef struct
{
 int mask;
 char* name;
 int wd;
} QEVENT;

int mkt(int argc, char* argv[]);
int dump(int argc, char* argv[]);
int client(int argc, char* argv[]);
int getcnt(char* in[]);

//BT signal
int stopbt();
int startbt();
void signalstop();


//#define MKTCMD(file,out)  { \
char* mkcmd[]={"x",file,"-o",out,TORE}; \
mkt(getcnt(mkcmd),mkcmd); \
}

#define DUMPCMD(file)  { \
char* mkcmd[]={"x",file,TORE}; \
dump(getcnt(mkcmd),mkcmd); \
}



#endif
