#include "torrent.h"
#include "FileSystem.h"
#include "stdint.h"
#include <curl/curl.h>
#include <jansson.h>
#include <stdio.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <p2ptorrent.h>

#define VB 0
extern char gUUID[16];
extern char gMetPath[10000];
extern char gMonpath[10000];

struct curlstring {
  char *ptr;
  size_t len;
};


std::string FileSystem::mId="";
std::string FileSystem::mRoot="";
std::string FileSystem::mMon="";
bool FileSystem::stop = false;
std::vector<QEVENT> FileSystem::mQ;
std::string FileSystem::mMeta = "";
int FileSystem::torCounter=0;

void  FileSystem::refreshFile(const char* fname, char* b64buffer)
{
   int outlen=0;
   FILE* fp=fopen(fname,"w");
   char* buffer = (char*)unbase64((unsigned char*)b64buffer, strlen(b64buffer),&outlen);
   fwrite(buffer,1,outlen,fp);
   fclose(fp);
}

char* FileSystem::loadFile(const char* fname,int* sz)
{
  char* buffer=NULL;
  FILE* fp =fopen(fname,"r");
  struct stat fs;
    if(stat(fname,&fs) < 0)    
        return NULL;
    else
    {
       *sz = fs.st_size;	
       buffer=(char*)malloc(fs.st_size);
       fread(buffer,1,fs.st_size,fp);  
    }
  fclose(fp);
  return buffer;
}


char* FileSystem::base64(const unsigned char *input, int length)
{
        BIO *bmem, *b64;
        BUF_MEM *bptr;

        b64 = BIO_new(BIO_f_base64());
        bmem = BIO_new(BIO_s_mem());
        b64 = BIO_push(b64, bmem);
        BIO_write(b64, input, length);
        BIO_flush(b64);
        BIO_get_mem_ptr(b64, &bptr);

        char *buff = (char *)malloc(bptr->length);
        memcpy(buff, bptr->data, bptr->length-1);
        buff[bptr->length-1] = 0;

        BIO_free_all(b64);

        return buff;
}


char* FileSystem::unbase64(unsigned char *input, int length,int* outlen)
        {
        BIO *b64, *bmem;

        char *buffer = (char *)malloc(length);
        memset(buffer, 0, length);

        b64 = BIO_new(BIO_f_base64());
        bmem = BIO_new_mem_buf(input, length);
        bmem = BIO_push(b64, bmem);

        *outlen = BIO_read(bmem, buffer, length);

        BIO_free_all(bmem);

        return buffer;
}



void FileSystem::init_string(struct curlstring *s) {
  s->len = 0;
  s->ptr =(char*) malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0'; 
}


int FileSystem::writefunc(void *ptr, int size, int nmemb, struct curlstring *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = (char*)realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;
   return size*nmemb;
}


int FileSystem::writefuncEvent(void *ptr, int size, int nmemb, struct curlstring *s)
{
  s->ptr=strstr((char*)ptr,"{");
  char* t = strstr((char*)ptr,"}}");
  s->len = t-s->ptr+2;

if(t!=NULL && s->ptr!=NULL) {
   s->ptr[s->len]='\0';
//  fprintf(stderr,"\n written [ %s ]",s->ptr);
  json_error_t error;
  json_t* root = json_loads(s->ptr, 0, &error);
  json_t* id = json_object_get(root, "data");
  json_t* val = json_object_get(id,"val");
  json_t* uid = json_object_get(id,"uid");

  char tmpuuid[16]={0};
  strcpy(tmpuuid,json_string_value(uid));
if(strcmp(tmpuuid, gUUID))
{
  printf("\n\n----Incoming meta Event... Got b64 meta.idx: %s ----- uuid %s GUUID %s ----", json_string_value(val),json_string_value(uid),gUUID);
  std::string  tpath = mMon + std::string("/") + std::string("meta.idx");
  char* buf = (char*)json_string_value(val);
  refreshFile(tpath.c_str(),buf);
  asyncsimplep2p((char*)tpath.c_str(), gMonpath );
}
else
 printf("\n ignoring local meta events....");  
}

  return size*nmemb;
}

/*# curl -d proto=gcm -d token=d0 http://localhost:8080/subscribers
{
  "proto": "gcm",
  "token": "d0",
  "updated": 1414160559,
  "created": 1414160559,
  "id": "bgLb_K5Xzs0"
root@skychat:/mnt/dev/qbt/x86/server/client# curl -X POST   http://localhost:8080/subscriber/bgLb_K5Xzs0/subscriptions/event
root@skychat:/mnt/dev/qbt/x86/server/client# curl http://localhost:8080/subscribe?events=event*/
#define EVTROOT "http://localhost:8080/subscribers"
#define EVTSUB(ID)  std::string("http://localhost:8080/subscriber/")+std::string(ID)+std::string("/subscriptions/signal")
#define EVTLISTEN "http://localhost:8080/subscribe?events=signal"
#define EVTPOST "http://localhost:8080/event/signal"

void FileSystem::initService()
{
    CURL* curl = curl_easy_init();
    struct curlstring s;
    init_string(&s);

    struct curl_slist *headers=NULL;

    std::string shinyurl=EVTROOT;
   // fprintf(stderr,"\n---url---- : %s",shinyurl.c_str());	

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, VB);
    curl_easy_setopt(curl, CURLOPT_URL, shinyurl.c_str());  
   // curl_easy_setopt(curl, CURLOPT_POST, 1L); /* !!! */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "proto=gcm&token=d0"); /* data goes here */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
   // printf("\n Response: %s \n", s.ptr);	


    json_error_t error;
    json_t* root = json_loads(s.ptr, 0, &error);
    json_t* id = json_object_get(root, "id");
    mId = json_string_value(id);
    fprintf(stderr,"\n id = %s",mId.c_str());
}
}

void FileSystem::subscribeService()
{
    CURL* curl = curl_easy_init();
    struct curlstring s;
    init_string(&s);
    std::string shinyurl=EVTSUB(FileSystem::mId);
   // fprintf(stderr,"\n---url---- : %s",shinyurl.c_str());

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, VB);
 //   curl_easy_setopt(curl, CURLOPT_POST, 1L); /* !!! */
    curl_easy_setopt(curl, CURLOPT_URL, shinyurl.c_str());
//    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
//    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
//    printf("\n Response: %s \n", s.ptr);
}
}


void FileSystem::postMetaEvent(char* meta,char* gUUID)
{
    CURL* curl = curl_easy_init();
    std::string shinyurl=EVTPOST;
  //  fprintf(stderr,"\n---url---- : %s",shinyurl.c_str());

  if (curl) {

    std::string data("data.val=");
    data+=std::string(meta);
    data+=std::string("&data.uid=");
    data+=std::string(gUUID);	
    curl_easy_setopt(curl, CURLOPT_VERBOSE, VB);
    curl_easy_setopt(curl, CURLOPT_POST, 1L); /* !!! */
    curl_easy_setopt(curl, CURLOPT_URL, shinyurl.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str()); /* data goes here */
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}
}


void FileSystem::listenEvent()
{
    	
    CURL* curl = curl_easy_init();
    struct curlstring s;
    init_string(&s);
    std::string shinyurl=EVTLISTEN;

  fprintf(stderr,"\n---listening p2p Events...from pushd url : %s",shinyurl.c_str());

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, VB);
    curl_easy_setopt(curl, CURLOPT_URL, shinyurl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefuncEvent);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
   // printf("\n Response: %s \n", s.ptr);
}
}



void FileSystem::unregister()
{
	stop=true;
    ( void ) inotify_rm_watch( fd, wd[0] );
    ( void ) close( fd );
}
void FileSystem::sendUpdates(QEVENT* event)
{

		//meta data torrents files
		std::string  tpath = mMon + std::string("/") + std::string("meta.idx");
		char* tpathcstr=(char*)malloc(tpath.size()*2);
		strcpy(tpathcstr,tpath.c_str());	
		int sz=0;	
		//final cmd			
		char* mkcmd[]={"","","-o",tpathcstr,TORE}; 
		mkcmd[1] = (char*)malloc(mMeta.size());
		strcpy(mkcmd[1],mMeta.c_str());
		mkt(getcnt(mkcmd),mkcmd); 
		printf("\nGenearting meta index...... %s..",tpath.c_str());
                char* buffer = loadFile((char*)tpath.c_str(), &sz);
		char* b64 = base64((const unsigned char*)buffer,sz);
	        printf("\n metafile %d / sz %s ",sz,b64);
                asyncsimplep2p((char*)tpath.c_str(),gMonpath);
                postMetaEvent(b64,gUUID);
}

void FileSystem::generateTorrents(QEVENT* event)
{
					//Geenrate Torrent only here is created.....
					std::string blobpath = mRoot+ std::string("/") +  std::string(event->name);
					{
                   	 	//printf("The file %s was created.\n", blobpath.c_str() );
						//Create update torrent
						//meta data torrents files
						std::string tpath = mMon + std::string("/meta/") + std::string(event->name) + std::string(".meta");
						char* tpathcstr=(char*)malloc(tpath.size()*2);
						strcpy(tpathcstr,tpath.c_str());		
						//final cmd			
						char* mkcmd[]={"","","-o",tpathcstr,TORE}; 
						mkcmd[1] = (char*)malloc(blobpath.size());
						strcpy(mkcmd[1],blobpath.c_str());
						mkt(getcnt(mkcmd),mkcmd); 
						
					}/*
					std::string metapath = mMon+ std::string("/meta/") ; //<root>/meta/  
					{
						//meta data torrents files
						std::string  tpath = mMon + std::string("/") + std::string("meta.idx");
						char* tpathcstr=(char*)malloc(tpath.size()*2);
						strcpy(tpathcstr,tpath.c_str());		
						//final cmd			
						char* mkcmd[]={"","","-o",tpathcstr,TORE}; 
						mkcmd[1] = (char*)malloc(metapath.size());
						strcpy(mkcmd[1],metapath.c_str());
						mkt(getcnt(mkcmd),mkcmd); 
						printf("\nGenearting meta index...... %s..",tpath.c_str());

					}	*/

}

void* FileSystem::execute_events(void*)
{
	while(!stop)
	{
		//FileSystem::dumpQ(); 
		usleep(1);
		if(!FileSystem::mQ.empty())
		{
			//printf("\n running....");
			QEVENT e = FileSystem::popEvent();
			QEVENT *event = &e; 
	            if ( event->mask & IN_CREATE ) {
	                if ( event->mask & IN_ISDIR ) {
	                    printf( "Event for %s : The directory %s was created.\n", (event->wd==wd[0])?"blob" :"meta",  event->name );       
	                }
	                else {
	                    printf( "Event for %s : The directory %s was created.\n", (event->wd==wd[0])?"blob" :"meta",  event->name );       
						
						if(event->wd == FileSystem::wd[0])//Blob event
						{
							FileSystem::generateTorrents(event);
							//if(++torCounter > 3)
							FileSystem::sendUpdates(event);
						}	
						if(event->wd == FileSystem::wd[1])//meta event
						{
							//Send update
						}	
													
	                }
	            } else
	            if ( event->mask & IN_DELETE ) {
	                if ( event->mask & IN_ISDIR ) {
	                    printf( "Event for %s : The directory %s was deleted.\n",  (event->wd==wd[0])?"blob" :"meta",  event->name );       
	                }
	                else {
	                    printf( "Event for %s : The file %s was deleted.\n", (event->wd==wd[0])?"blob" :"meta",  event->name );
						
						if(event->wd == FileSystem::wd[0])//Blob event
						{

						}	
						if(event->wd == FileSystem::wd[1])//meta event
						{
							//Send update
						}	
	                }
	            } if ( event->mask & IN_MODIFY ) {
	                if ( event->mask & IN_ISDIR ) {
	                    printf( "Event for %s : The directory %s was modified.\n", (event->wd==wd[0])?"blob" :"meta",  event->name);       
	                }
	                else {
	                    printf( "Event for %s :The file %s was modified.\n", (event->wd==wd[0])?"blob" :"meta",  event->name);
						
						if(event->wd == FileSystem::wd[0])//Blob event
						{

						}	
						if(event->wd == FileSystem::wd[1])//meta event
						{
							//Send update
						}	
	                }
	            } else
	            if ( event->mask & IN_MODIFY ) {
	                if ( event->mask & IN_ISDIR ) {
	                    printf( "Event for %s : The directory %s was changed.\n",  (event->wd==wd[0])?"blob" :"meta",  event->name);       
	                }
	                else {
	                    printf( "Event for %s : The file %s was changed.\n",  (event->wd==wd[0])?"blob" :"meta",  event->name);
						
						if(event->wd == FileSystem::wd[0])//Blob event
						{

						}	
						if(event->wd == FileSystem::wd[1])//meta event
						{
							//Send update
						}	
					
	                }
	            } 
	        }
	}
	
}

void FileSystem::dumpQ()
{
	//printf("\n Q sz= %d",mQ.size());
	for(std::vector<QEVENT>::iterator it = mQ.begin(); it!=mQ.end();++it)
		printf("\n mQ : %s mask: %d",it->name, it->mask);
}


void* FileSystem::process_events(void*)
{ 
    int length, i = 0;
    char buffer[BUF_LEN];

    fd = inotify_init(); 

    if ( fd < 0 ) {
        perror( "inotify_add_watch" );
    }

	//Blob watcher
    wd[0] = inotify_add_watch( fd, mRoot.c_str(), IN_ALL_EVENTS); //IN_CREATE);
  	//Meta watcher
    wd[1] = inotify_add_watch( fd, mMeta.c_str(), IN_ALL_EVENTS); //IN_CREATE);

    while (!stop){
        struct inotify_event *event;

        length = read( fd, buffer, BUF_LEN );  

        if ( length < 0 ) { 
            perror( "read" );
        } 

        event = ( struct inotify_event * ) &buffer[ i ];
        if ( event->len ) {
                    //printf( "IN Event %s mask %d.\n", event->name, event->mask );       
					QEVENT e;
					e.name=(char*)malloc(strlen(event->name));
					strcpy(e.name,event->name);	
					e.mask = event->mask;
					e.wd = event->wd;
					FileSystem::addEvent(e)	;

					//Cross checks...	
					
		}			
	
       
    }	
}
	
