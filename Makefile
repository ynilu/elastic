# the compiler: gcc for C program, define as g++ for C++
PP = g++
RM = rm -f
# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS = -g -Wall -O3 -Wno-c++11-extensions -std=c++11

# the option to select USNET or NSFNET, USNET is the default
# DNSF = -D NSF

# the build target executable:
# TARGET = simulator

all: simulator.o graph.o traffic.o auxiliary.o
	$(PP) $(CFLAGS) -o run_simulator simulator.o graph.o traffic.o auxiliary.o

simulator.o: simulator.cpp
	$(PP) $(CFLAGS) -c simulator.cpp

graph.o: graph.cpp
	$(PP) $(CFLAGS) -c graph.cpp

traffic.o: traffic.cpp
	$(PP) $(CFLAGS) -c traffic.cpp

auxiliary.o: auxiliary.cpp
	$(PP) $(CFLAGS) -c auxiliary.cpp

clean:
	$(RM) run_simulator *.o


