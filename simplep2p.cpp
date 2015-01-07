/*

Copyright (c) 2003, Arvid Norberg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdlib.h>
#include "libtorrent/peer_info.hpp"
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/torrent_handle.hpp"
#include <pthread.h>    /* POSIX Threads */
#include <p2ptorrent.h>

pthread_t infop2p;
pthread_attr_t attrp2p ;
void* simplep2p(void*);


P2Pdata d;

int asyncsimplep2p(char* torrent,char* savepath)
{
    int s=0;
    if( (s = pthread_attr_init(&attrp2p)) !=0)
    {
                fprintf(stderr,"\n Err init simplep2p");
                return -1;
    }
    d.savepath = (char*)malloc(strlen(savepath));;
    d.torrent = (char*)malloc(strlen(torrent));
   strcpy(d.savepath,savepath);
   strcpy(d.torrent,torrent);
   fprintf(stderr,"+++++++ %s++++   %s \n",d.torrent,d.savepath);

    pthread_create(&infop2p, &attrp2p, &simplep2p, (void*)&d);

//    pthread_join(info,NULL);
    return 0;
}

void* simplep2p(void* pp)
{
	using namespace libtorrent;
	P2Pdata* p2pdata = (P2Pdata*)pp;
	char torrent[1000]; char savepath[1000];

	strcpy(torrent,p2pdata->torrent);
	strcpy(savepath,p2pdata->savepath);
	fprintf(stderr,"\n\n------------ %s ----%s \n",p2pdata->savepath,p2pdata->torrent);
	session s;
	error_code ec;
	s.listen_on(std::make_pair(6881, 6889), ec);
	if (ec)
	{
		fprintf(stderr, "failed to open listen socket: %s\n", ec.message().c_str());
		//return 1;
	}
	add_torrent_params p;
	p.save_path = std::string(savepath);
	p.ti = new torrent_info(torrent, ec);
	if (ec)
	{
		fprintf(stderr, "%s\n", ec.message().c_str());
		//return 1;
	}
	torrent_handle h = s.add_torrent(p, ec);
	if (ec)
	{
		fprintf(stderr, "%s\n", ec.message().c_str());
	//	..return 1;
	}

	// wait for the user to end
	//char a;
	//scanf("%c\n", &a);
	/* torrent_info const& info = h.get_torrent_info();
                                for (int i = 0; i < info.num_files(); ++i)
                                {
                                        bool pad_file = info.file_at(i).pad_file;
                                        if (!show_pad_files && pad_file) continue;
                                        int progress = info.file_at(i).size > 0
                                 */
	std::vector<size_type> progress;
	std::vector<peer_info> pinfo;
	torrent_info const& info = h.get_torrent_info();
	int prev =0;
	int prevstate =-1;
	while(1)
	{
		h.file_progress(progress);
		torrent_status s = h.status();
		h.get_peer_info(pinfo);
	 	
		fprintf(stderr,"\n\r status : %d state %d  sz %d ",progress[0],s.state, pinfo.size()); 
		if(prev > 0 && pinfo.size()==0 || (s.state == s.seeding && prevstate == s.downloading ))
		  break;		
		prev = pinfo.size();
		prevstate = s.state;
		usleep(10);
	}

//	return 0;
}


