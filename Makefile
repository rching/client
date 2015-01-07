CC=gcc
CPP=g++

ifndef V
QUIET_CC = @printf '    %b %b\n' $(CCCOLOR)CC$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_LINK = @printf '    %b %b\n' $(LINKCOLOR)LINK$(ENDCOLOR) $(BINCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_INSTALL = @printf '    %b %b\n' $(LINKCOLOR)INSTALL$(ENDCOLOR) $(BINCOLOR)$@$(ENDCOLOR) 1>&2;
endif

LJSCC=$(CPP)

TOP ?= $(shell pwd)
BPS_ROOT = $(TOP)/../
LJS_ROOT = $(TOP)/
THR_ROOT = /mnt/dev/projects/3rd/install



DEFS = -DPACKAGE_NAME=\"libtorrent-rasterbar\" -DPACKAGE_TARNAME=\"libtorrent-rasterbar\" -DPACKAGE_VERSION=\"0.16.15\" -DPACKAGE_STRING=\"libtorrent-rasterbar\ 0.16.15\" -DPACKAGE_BUGREPORT=\"arvid@rasterbar.com\" -DPACKAGE_URL=\"http://www.libtorrent.org\" -DPACKAGE=\"libtorrent-rasterbar\" -DVERSION=\"0.16.15\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\" -DHAVE_PTHREAD=1 -DHAVE_BOOST=/\*\*/ -DHAVE_BOOST_SYSTEM=1 -DHAVE_GETHOSTBYNAME=1 -DHAVE_GETHOSTBYNAME_R=1 -DGETHOSTBYNAME_R_RETURNS_INT=1 -DHAVE_CLOCK_GETTIME=1 -DNDEBUG=1 -DTORRENT_USE_OPENSSL=1 -DHAVE_LINUX_TYPES_H=1 -DHAVE_LINUX_FIEMAP_H=1 -DWITH_SHIPPED_GEOIP_H=1 -DBOOST_ASIO_HASH_MAP_BUCKETS=1021 -DBOOST_EXCEPTION_DISABLE=1 -DBOOST_ASIO_ENABLE_CANCELIO=1 -DBOOST_ASIO_DYN_LINK=1 -DTORRENT_BUILDING_SHARED=1 
INCS = -I. -I$(THR)/include  -I/usr/include  -I./libtorrent-rasterbar-0.16.15/include  -I./include/ -I/mnt/dev/qbt/x86/server/client/install/include
CFLAGS= $(DEFS) -g -O2 -ftemplate-depth=120 -fvisibility-inlines-hidden -fvisibility=hidden -Wno-literal-suffix -Wno-unused-result -Wno-write-strings -Wno-deprecated-declarations -Wno-unused-parameter  $(INCS) -fPIC
LIBS = -ltorrent-rasterbar  -lcurl -ldl -lm -lc -lboost_system  -lssl -lcrypto -lpthread -ljansson 
LDFLAGS= -L$(THR)/lib  -L/mnt/dev/qbt/x86/server/client/install/lib  $(LIBS) 



#AGENT object files
OBJ_FILES := $(addprefix obj/,$(notdir $(BT_SRC):.cpp=.o)))  



#
all : Main


clean :
	rm -rf obj/*
	rm ihog

Main: agent $(shell mkdir -p obj/) $(shell mkdir -p bin/)
	echo "done"


obj/client_test.o : $(BT_SRC) $(IPP)
	$(LJSCC) $(CFLAGS) $(BT_SRC) $(IPP)  -c $< -o $@

BT_SRC := ./src/client_test.cpp

IPP := ./install/include/boost/asio/ip/impl/address_v4.ipp \


MAIN_SRC := main.cpp \
			FileSystem.cpp \
			make_torrent.cpp \
			dump_torrent.cpp \
			simplep2p.cpp
			

	
#mkt: make_torrent.cpp
#	$(CPP)   $(CFLAGS) make_torrent.cpp -o mkt $(LDFLAGS)

simple : simple.cpp
	$(CPP)   $(CFLAGS) simple.cpp -o simple $(LDFLAGS)

client : client.cpp
	$(CPP)   $(CFLAGS) client.cpp -o client $(LDFLAGS)
	

agent: $(MAIN_SRC) obj/client_test.o
	$(LJSCC)   $(CFLAGS) $(MAIN_SRC)  obj/client_test.o   -o ihog $(LDFLAGS)

