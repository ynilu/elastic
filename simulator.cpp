#include "graph.h"

int main(int argc, char *argv[])
{
    graph_info info;
    info.graph = "USnet.txt";
    info.source = "USnet_source.txt";
    info.traffic = "USnet_traffic.txt";
    info.num_of_slots = 200;
    phy_graph test (info);

    return 0;
}






