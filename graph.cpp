#include "graph.h"

using namespace std;

phy_graph::phy_graph(graph_info &g_info)
{
    read_network_file(g_info.graph_file, g_info.num_slots);
}

phy_graph::~phy_graph()
{

}

phy_node& phy_graph::get_node(int id)
{
    return node[id];
}

phy_link& phy_graph::get_link(int source, int destination)
{
    return link[make_pair(source,destination)];
}
void phy_graph::read_network_file(char* graph_file, int num_slots)
{
    int num_nodes;
    fstream fs;

    fs.open(graph_file, ios::in);
    if(!fs)
    {
        cerr << "Can't open file\"" << graph_file << "\"\n";
        cerr << "Please give network node file, program aborting...\n";
        exit(1);
    }

    fs >> num_nodes;

    // resize vectors according to the number of nodes
    node.resize(num_nodes);

    int node_a, node_b, distance;
    while(fs >> node_a >> node_b >> distance)
    {
        node_a--;
        node_b--;

        node[node_a].degree++;
        node[node_b].degree++;

        node[node_a].neighbor.push_back(node_b);
        node[node_b].neighbor.push_back(node_a);

        phy_link new_link;

        new_link.source = node_a;
        new_link.destination = node_b;
        new_link.distance = distance;
        new_link.slot.resize(num_slots);

        link[make_pair(node_a,node_b)] = new_link;

        new_link.source = node_b;
        new_link.destination = node_a;

        link[make_pair(node_b,node_a)] = new_link;
    }

    fs.close();
}


phy_node::phy_node()
{

}

phy_node::~phy_node()
{

}

phy_link::phy_link()
{

}

phy_link::~phy_link()
{

}

transmitter::transmitter()
{

}

transmitter::~transmitter()
{

}

receiver::receiver()
{

}

receiver::~receiver()
{

}
