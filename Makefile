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