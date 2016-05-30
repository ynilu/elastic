#include "graph.h"

using namespace std;

Phy_graph::Phy_graph(Graph_info &g_info)
{
    read_network_file(g_info.graph_file, g_info.num_slots);
}

Phy_graph::~Phy_graph()
{

}

void Phy_graph::find_candidate_path()
{
    int num_nodes = node_list.size();
    set<bool> visited;
    queue<int> BFSQ;
}

Spectrum_result Phy_graph::find_best_spectrum(Path& path, int require_slots)
{

}

double Phy_graph::get_spectrum_weight(Path& path, int slot_st, int slot_ed)
{
    for(int i = 0; i < path.size() - 1; i++)
    {
        int from = path[i];
        int to   = path[i + 1];
    }
}

bool Phy_graph::spectrum_available(int from, int to, int slot_st, int slot_ed);
{
    for(int i = slot_st; i <= slot_ed; i++)
    {
        if(get_link(from, to).slot[i] != -1)
        {
            return false;
        }
    }
    return true;
}

Phy_node& Phy_graph::get_node(int id)
{
    return node_list[id];
}

Phy_link& Phy_graph::get_link(int source, int destination)
{
    return link_list[make_pair(source,destination)];
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
    node_list.resize(num_nodes);

    int node_a, node_b, distance;
    while(fs >> node_a >> node_b >> distance)
    {
        node_a--;
        node_b--;

        node_list[node_a].degree++;
        node_list[node_b].degree++;

        node_list[node_a].neighbor.push_back(node_b);
        node_list[node_b].neighbor.push_back(node_a);

        Phy_link new_link;

        new_link.source = node_a;
        new_link.destination = node_b;
        new_link.distance = distance;
        new_link.slot.resize(num_slots, 0);

        link_list[make_pair(node_a,node_b)] = new_link;

        new_link.source = node_b;
        new_link.destination = node_a;

        link_list[make_pair(node_b,node_a)] = new_link;
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
