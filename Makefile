CC ?= gcc
CXX ?= g++

TARGETDIR = ./
#TARGETDIR = ../bin

#CFCPU = -march=native
CFOPT = -O2 -finline -funroll-loops

CFDEBUG = -ggdb3
#CFDEBUG += -pg
#CFDEBUG += -DPROFILE
#CFWARN += -Wall
#CFWARN += -Wextra

#LDFLAGS = -s

# -------
#
# -------
TARGET_IRC=$(TARGETDIR)/seeborg-irc
TARGET_LINEIN=$(TARGETDIR)/seeborg-linein
TARGET_OBJS=seeborg.o seeutil.o utf8.o 
TARGET_HDRS=seeborg.h seeutil.h utf8.h required.h
TARGET_LINEIN_OBJS=seeborg-linein.o
TARGET_IRC_OBJS=seeborg-irc.o
TARGET_IRC_HDRS=seeborg-irc.h
# botnet library
TARGET_IRC_OBJS+=botnet/botnet.o botnet/dcc_chat.o botnet/dcc_send.o
TARGET_IRC_OBJS+=botnet/output.o botnet/server.o botnet/utils.o
TARGET_IRC_HDRS+=botnet/botnet.h botnet/includes.h

## Linux 
ifeq ($(shell uname), Linux)
CFPLATFORM += -DHAVE_WCSDUP -DHAVE_WCSNCASECMP
CFPLATFORM += -pthread
endif

## FreeBSD 
ifeq ($(shell uname), FreeBSD)
CFPLATFORM += -DHAVE_WCSDUP
CFPLATFORM += -pthread
endif

## MacOSX 
ifeq ($(shell uname), Darwin)
CFPLATFORM += -pthread
endif

## Windows (mingw) 
ifeq ($(shell uname), MINGW32_NT-5.1)
LDFLAGS += -lwsock32
CFPLATFORM +=-DHAVE_WCSDUP -DHAVE_WCSNCASECMP
endif

CFLAGS = $(CFWARN) $(CFCPU) $(CFOPT) $(CFDEBUG) $(CFUSER) $(CFPLATFORM)
CXXFLAGS = $(CFLAGS)


all: compile

clean:
	rm -f $(TARGET_IRC) $(TARGET_LINEIN) $(TARGET_OBJS) $(TARGET_IRC_OBJS) $(TARGET_LINEIN_OBJS)

compile: makedirs $(TARGET_LINEIN) $(TARGET_IRC)

$(TARGET_IRC): $(TARGET_OBJS) $(TARGET_IRC_OBJS) $(TARGET_HDRS) $(TARGET_IRC_HDRS)
	@echo Linking $@...
	@$(CXX) $(CXXFLAGS) $(TARGET_OBJS) $(TARGET_IRC_OBJS) -o $@ $(LDFLAGS)

$(TARGET_LINEIN): $(TARGET_OBJS) $(TARGET_LINEIN_OBJS) $(TARGET_HDRS) $(TARGET_LINEIN_HDRS)
	@echo Linking $@...
	@$(CXX) $(CXXFLAGS) $(TARGET_OBJS) $(TARGET_LINEIN_OBJS) -o $@ $(LDFLAGS)

.cpp.o:
	@echo Compiling $@...
	@$(CXX) -c $< -o $@ $(CXXFLAGS)

.c.o:
	@echo Compiling $@...
	@$(CC) -c $< -o $@ $(CFLAGS)

makedirs:
	@if [ ! -d $(TGTDIR) ];then mkdir $(TGTDIR);fi
