#ifndef _SIMPLEP2P
#define _SIMPLEP2P
typedef struct
{
  char* savepath;
  char* torrent;
} P2Pdata;

int asyncsimplep2p(char* torrent,char* savepath);

#endif
