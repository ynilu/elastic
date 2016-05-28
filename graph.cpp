#include "graph.h"

using namespace std;

Phy_graph::Phy_graph(Graph_info &g_info)
{
    read_network_file(g_info.graph_file, g_info.num_slots);
}

Phy_graph::~Phy_graph()
{

}

Phy_node& Phy_graph::get_node(int id)
{
    return node[id];
}

Phy_link& Phy_graph::get_link(int source, int destination)
{
    return link[make_pair(source,destination)];
}
void Phy_graph::read_network_file(char* graph_file, int num_slots)
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

        Phy_link new_link;

        new_link.source = node_a;
        new_link.destination = node_b;
        new_link.distance = distance;
        new_link.slot.resize(num_slots, 0);

        link[make_pair(node_a,node_b)] = new_link;

        new_link.source = node_b;
        new_link.destination = node_a;

        link[make_pair(node_b,node_a)] = new_link;
    }

    fs.close();
}


Phy_node::Phy_node()
{

}

Phy_node::~Phy_node()
{

}

Phy_link::Phy_link()
{

}

Phy_link::~Phy_link()
{

}

Transmitter::Transmitter()
{

}

Transmitter::~Transmitter()
{

}

Receiver::Receiver()
{

}

Receiver::~Receiver()
{

}
