#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <sys/wait.h>   /* Wait for Process Termination */
#include <assert.h>
#include <stdio.h>          // For printf()
#include <stdlib.h>         // For exit() etc.
#include <string.h>         // For strlen() etc.
#include <unistd.h>         // For select()
#include <errno.h>          // For errno, EINTR
#include <signal.h>
#include <fcntl.h>
#include "FileSystem.h"

#include "torrent.h"

using namespace std;

//Globals
int FileSystem::fd =0;
int FileSystem::wd[]={0};
pthread_t btinfo,btmetainfo;
pthread_attr_t btattr,btmetaattr;

pthread_t tinfo,ptinfo;
pthread_attr_t attr,pattr;



char gSavepath[10000]="/tmp";  //blob
char gMonpath[10000]="/tmp";   //root  
char gMetPath[10000]="/tmp";   //meta path
char gMindex[10000]="";
char gUUID[16]={0};
//File system monitor
int startmonitor()
{
    int s=0;
        
    if( (s = pthread_attr_init(&attr)) !=0)
    {	
		fprintf(stderr,"\n Err init monitor");
		return -1;
    }
    pthread_create(&tinfo, &attr, &FileSystem::process_events, NULL);
	
    if( (s = pthread_attr_init(&pattr)) !=0)
    {	
		fprintf(stderr,"\n Err init monitor");
		return -1;
    }
    pthread_create(&ptinfo, &attr, &FileSystem::execute_events, NULL);
 
	
    return 0;
}



void* btclient(void* params)
{
		char* file = (char*)params;
		char* cmd[]={"clientcmd","-m",gMetPath,"-s",gSavepath,TORE};
		client(getcnt(cmd),cmd); 
}

void* btmetaclient(void* params)
{
		char* file = (char*)params;
		char* cmd[]={"clientcmd",gMindex,"-s",gMetPath,TORE};
		client(getcnt(cmd),cmd); 

}


int startbt()
{
    int s=0;     
    if( (s = pthread_attr_init(&btattr)) !=0)
    {	
		fprintf(stderr,"\n Err init bt");
		return -1;
    }
    pthread_create(&btinfo, &btattr, &btclient, (void*)NULL);
    return 0;
}

int startbtmeta()
{
    int s=0;
        
    if( (s = pthread_attr_init(&btmetaattr)) !=0)
    {	
		fprintf(stderr,"\n Err init bt");
		return -1;
    }
    pthread_create(&btmetainfo, &btmetaattr, &btmetaclient, (void*)NULL);
    return 0;
}

int stopbt()
{
	signalstop();
	//stop Bt
	pthread_join(btinfo,NULL);
	pthread_join(btmetainfo,NULL);
	//stop monitors
	pthread_join(tinfo,NULL);
	pthread_join(ptinfo,NULL);
}



//Signal handler 
static void HandleSignals(int sigraised)
{
    assert(sigraised == SIGINT);
    fprintf(stderr,"User SIG Ctrl C exit....");
	FileSystem::getInstance().unregister();
    exit(0);
}


//Helper
int getcnt(char** in)
{
	int i=0;
	while(strcmp(TORE,in[i]))
		i++;
	printf("\n cnt=%d",i);
	return i;
}



//Main app
int main(int argc, char** argv) {
 
    signal(SIGINT, HandleSignals);     // SIGQUIT is what you get for a Ctrl-\ (indeed)
	if(argc < 3)
	{
		printf("\n ./ihog <blob path> <meta root paths> \n");
		return -1;
	}
	strcpy(gSavepath,argv[1]);
	strcpy(gMonpath,argv[2]);
	strcpy(gUUID,argv[3]);
	strcpy(gMetPath,gMonpath);
	strcat(gMetPath,"/meta/");
	strcpy(gMindex,gMonpath);
	strcat(gMindex,"/meta.idx");
	
	std::string INITCMD("mkdir -p ");
	INITCMD+=std::string(gMetPath);
	
	system(INITCMD.c_str());
	
	printf("\n Root: %s Index: %s \n\n",gMonpath,gMindex);

	FileSystem::init(std::string(gSavepath) ,  std::string(gMonpath));
	
	
	//Register
	FileSystem::initService();
	FileSystem::subscribeService();
	
	//Start FileSystem Monitor thread
	startmonitor();

	//Listen event loog here
	//FileSystem::listenEvent();

	//Start File monitor 
	// BT main start
	//startbt();		
		
	
    //Dummy loop keep daemon running
	while(true)
	{
           FileSystem::listenEvent();
	   usleep(1000);	
	}

    return 0;
}

