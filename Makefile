# the compiler: gcc for C program, define as g++ for C++
CXX ?= g++
RM = rm -f
# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS = -g -Wall -O3 -std=c++11

# the option to select USNET or NSFNET, USNET is the default
# DNSF = -D NSF

# the build target executable:
# TARGET = simulator

all: simulator.o graph.o traffic.o auxiliary.o light_path.o spectrum.o
	$(CXX) $(CFLAGS) -o run_simulator simulator.o graph.o traffic.o auxiliary.o light_path.o spectrum.o

simulator.o: simulator.cpp
	$(CXX) $(CFLAGS) -c simulator.cpp

graph.o: graph.cpp graph.hpp
	$(CXX) $(CFLAGS) -c graph.cpp

traffic.o: traffic.cpp traffic.hpp
	$(CXX) $(CFLAGS) -c traffic.cpp

auxiliary.o: auxiliary.cpp auxiliary.hpp
	$(CXX) $(CFLAGS) -c auxiliary.cpp

light_path.o: light_path.cpp light_path.hpp
	$(CXX) $(CFLAGS) -c light_path.cpp

spectrum.o: spectrum.cpp spectrum.hpp
	$(CXX) $(CFLAGS) -c spectrum.cpp

clean:
	$(RM) run_simulator *.o


