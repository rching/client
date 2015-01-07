#ifndef _FS
#define FS
#include<iostream>
#include<vector>
#include<cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include "pthread.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>



#include <ifaddrs.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <curl/curl.h>
#include <jansson.h>
#include "torrent.h"




#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


//public api

class FileSystem
{
private:
    static int fd;
    static int wd[2];
	static bool stop;
	static int torCounter;
    static std::string mId; 


    FileSystem() {}

public:

        static FileSystem& getInstance()
        {
          static FileSystem instance;
          return instance;
        }
		
	static std::string mRoot; //Blob data root ...
	static std::string mMon;  //meta data root
	static std::string mMeta;  //meta data directory
	
	static std::vector<QEVENT> mQ;
		
		
//helpers
static void* process_events(void*);
static void* execute_events(void*);
static void dumpQ();
static void generateTorrents(QEVENT* event);
static void sendUpdates(QEVENT* event);

static void init_string(struct curlstring *s);
static int writefunc(void *ptr, int size, int nmemb, struct curlstring *s);
static int writefuncEvent(void *ptr, int size, int nmemb, struct curlstring *s);
static void initService();
static void subscribeService();
static void listenEvent();
static void postMetaEvent(char* meta,char* uuid);
static char* loadFile(const char* fname, int* sz);
void unregister();

static char* unbase64(unsigned char *input, int length,int* outlen);
static char* base64(const unsigned char *input, int length);
static void refreshFile(const char* fname, char* b64buffer);

static void addEvent(QEVENT e)
{
	//remove duplicated events if there is one...
	for(std::vector<QEVENT>::iterator it = mQ.begin(); it!=mQ.end();++it)
		if((it->mask == e.mask )&& !strcmp(it->name,e.name))
			return;
	mQ.push_back(e);
}

static QEVENT popEvent()
{
	QEVENT x = mQ.back();
	mQ.pop_back();
	return x;
}


static void init(std::string root,std::string monpath)
{
	mRoot=root;
	mMon=monpath;
	mMeta = mMon + std::string("/meta/");
	stop =false;
}


void execcmd(std::string cmd, std::string *res)
{
 fprintf(stderr,"\ncmd--: %s ",cmd.c_str()); 
 FILE* f = (FILE*)popen(cmd.c_str(),"r");
  char buf[10];
  if(f) {
   fscanf(f,"%s",buf);
   *res = buf;
  }
}

};

	

#endif

