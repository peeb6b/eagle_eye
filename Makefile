<<<<<<< HEAD
###############################################################################
# Programmer :  Paul Eggler
# Date :		03/28/2012
###############################################################################

.PHONY: all clean

ARCHIVE = Project
CXX = /usr/bin/g++
CXXFLAGS = -g -Wall -W -pedantic-errors

SOURCE_DIR = "/src"

CLIENTSOURCES = myTCP.c myNetwork.c echo.c client.c
SERVERSOURCES = myTCP.c myNetwork.c echo.c server.c
DRIVERSOURCES = myTCP.c myNetwork.c echo.c driver.c

CLIENTFILES = $(filter %.c, $(CLIENTSOURCES))
SERVERFILES = $(filter %.c, $(SERVERSOURCES))
DRIVERFILES = $(filter %.c, $(DRIVERSOURCES))

HEADERS =  $(wildcard *.h)

BPATCH = $(OTHERDIR)
CLIENTOBJECTS = $(CLIENTSOURCES:%.c=%.o)
SERVEROBJECTS = $(SERVERSOURCES:%.c=%.o)
DRIVEROBJECTS = $(DRIVERSOURCES:%.c=%.o)

default: client server driver

%.o: %.c
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

client: $(CLIENTOBJECTS)
	@echo "Building $@"
	@$(CXX) $(CXXFLAGS) $(CLIENTOBJECTS) -o $@
	@echo ""
	@echo "$@ worked :-) "
	@echo ""
	
server: $(SERVEROBJECTS)
	@echo "Building $@"
	@$(CXX) $(CXXFLAGS) $(SERVEROBJECTS) -o $@
	@echo ""
	@echo "$@ worked :-) "
	@echo ""

driver: $(DRIVEROBJECTS)
	@echo "Building $@"
	@$(CXX) $(CXXFLAGS) $(DRIVEROBJECTS) -o $@
	@echo ""
	@echo "$@ worked :-) "
	@echo ""

# target: clean
clean:
	-@rm -f $(SERVEROBJECTS) $(CLIENTOBJECTS) core client server

# target: dist
dist:
	@echo "tarring $(ARCHIVE)"
	tar -zcf $(ARCHIVE).tar --exclude '*.o' --exclude '*.tar'  . 
	
# Automatically generate dependencies and include them in Makefile
# target: depend
depend: $(CLIENTSOURCES) $(SERVERSOURCES) $(HEADERS)
	@echo "Generating dependencies"
	@$(CXX) -MM *.c > $@


-include depend
# Put a dash in front of include when using gnu make.
# It stops gmake from warning us that the file
# doesn't exist yet, even though it will be properly
# made and included when all is said and done.
=======
#
#

TGTDIR = ./
#TGTDIR = ../bin
FNAMEIRC = seeborg-irc
FNAMELINEIN = seeborg-linein

CFCPU = -march=pentium
CFOPT = -O3 -fomit-frame-pointer -fforce-addr -finline -funroll-loops -fexpensive-optimizations
CFUSER = -pthread

#CFDEBUG = -g3
#CFDEBUG += -pg
#LDFLAGS = -s
LDFLAGS = -lwsock32

SRCS = seeborg.cpp seeutil.cpp

# -------
#
# -------
CC = gcc
CXX = g++
CFLAGS = $(CFCPU) $(CFOPT) $(CFDEBUG) $(CFUSER)
CXXFLAGS = $(CFLAGS)

SRC_IRC = $(FNAMEIRC).cpp botnet/botnet.c botnet/dcc_chat.c botnet/dcc_send.c botnet/output.c \
    botnet/server.c botnet/utils.c
SRC_LINEIN = $(FNAMELINEIN).cpp

TGT_IRC = $(TGTDIR)/$(FNAMEIRC)
TGT_LINEIN = $(TGTDIR)/$(FNAMELINEIN)

OBJ_IRCTMP = $(SRC_IRC:%.cpp=%.o)
OBJ_IRC = $(OBJ_IRCTMP:%.c=%.o)

OBJ_LINEINTMP = $(SRC_LINEIN:%.cpp=%.o)
OBJ_LINEIN = $(OBJ_LINEINTMP:%.c=%.o)

DEP_IRCTMP = $(SRC_IRC:%.cpp=%.d)
DEP_IRC = $(DEP_IRCTMP:%.c=%.d)

DEP_LINEINTMP = $(SRC_LINEIN:%.cpp=%.d)
DEP_LINEIN = $(DEP_LINEINTMP:%.c=%.d)

OBJSTMP = $(SRCS:%.cpp=%.o)
OBJS = $(OBJSTMP:%.c=%.o)

DEPSTMP = $(SRCS:%.cpp=%.d)
DEPS = $(DEPSTMP:%.c=%.d)

DEPS += $(DEP_LINEIN) $(DEP_IRC)

all: compile

clean:
	rm -f $(TGT_IRC) $(TGT_LINEIN) $(OBJS) $(OBJ_IRC) $(OBJ_LINEIN)
# $(DEPS)

compile: makedirs $(TGT_LINEIN) $(TGT_IRC)

$(TGT_IRC): $(OBJS) $(OBJ_IRC)
	@echo Linking $@...
	$(CXX) $(CXXFLAGS) $(OBJS) $(OBJ_IRC) -o $@ $(LDFLAGS)

$(TGT_LINEIN): $(OBJS) $(OBJ_LINEIN)
	@echo Linking $@...
	$(CXX) $(CXXFLAGS) $(OBJS) $(OBJ_LINEIN) -o $@ $(LDFLAGS)


.cpp.d:
	@echo Updating $@...
	@$(CXX) $(CXXFLAGS) -MM $< -o $@

.c.d:
	@echo Updating $@...
	@$(CC) $(CFLAGS) -MM $< -o $@

.cpp.o:
	@echo Compiling $@...
	@$(CXX) -c $< -o $@ $(CXXFLAGS)

.c.o:
	@echo Compiling $@...
	@$(CC) -c $< -o $@ $(CFLAGS)

makedirs:
	@if [ ! -d $(TGTDIR) ];then mkdir $(TGTDIR);fi
>>>>>>> f0e2bedaaa15f2ed4bfe6b03991e3fa140f57d5a
