# the compiler: gcc for C program, define as g++ for C++
CXX ?= g++
RM = rm -f
# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS = -g -Wall -O3 -Wno-c++11-extensions -std=c++11

# the option to select USNET or NSFNET, USNET is the default
# DNSF = -D NSF

# the build target executable:
# TARGET = simulator

all: simulator.o graph.o traffic.o auxiliary.o light_path.o spectrum.o
	$(CXX) $(CFLAGS) -o run_simulator simulator.o graph.o traffic.o auxiliary.o light_path.o spectrum.o

simulator.o: simulator.cpp
	$(CXX) $(CFLAGS) -c simulator.cpp

graph.o: graph.cpp
	$(CXX) $(CFLAGS) -c graph.cpp

traffic.o: traffic.cpp
	$(CXX) $(CFLAGS) -c traffic.cpp

auxiliary.o: auxiliary.cpp
	$(CXX) $(CFLAGS) -c auxiliary.cpp

light_path.o: light_path.cpp
	$(CXX) $(CFLAGS) -c light_path.cpp

spectrum.o: spectrum.cpp
	$(CXX) $(CFLAGS) -c spectrum.cpp

clean:
	$(RM) run_simulator *.o


