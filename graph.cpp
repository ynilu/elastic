#include "graph.h"

phy_graph::phy_graph(graph_info &info)
{
    read_network_file(info.graph, info.num_of_slots);
    read_source_file(info.source);
    read_traffic_file(info.traffic);
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
void phy_graph::read_network_file(char* graph, int num_of_slots)
{
    int number_of_nodes;
    fstream fs;

    fs.open(graph, ios::in);
    if(!fs)
    {
        cerr << "Can't open file\"" << graph << "\"\n";
        cerr << "Please give network node file, program aborting...\n";
        exit(1);
    }

    fs >> number_of_nodes;

    // resize vectors according to the number of nodes
    node.resize(number_of_nodes);
    source_matrix.resize(number_of_nodes);
    num_dest_matrix.resize(number_of_nodes);
    traffic_matrix.resize(number_of_nodes);
    for(auto &element : traffic_matrix)
    {
        element.resize(number_of_nodes);
    }

    int node_a, node_b, distance;
    while(fs >> node_a >> node_b >> distance)
    {
        node_a--;
        node_b--;

        node[node_a].degree++;
        node[node_b].degree++;

        node[node_a].neighbour.push_back(node_b);
        node[node_b].neighbour.push_back(node_a);

        phy_link new_link;

        new_link.source = node_a;
        new_link.destination = node_b;
        new_link.distance = distance;
        new_link.slot.resize(num_of_slots);

        link[make_pair(node_a,node_b)] = new_link;

        new_link.source = node_b;
        new_link.destination = node_a;

        link[make_pair(node_b,node_a)] = new_link;
    }

    fs.close();
}

void phy_graph::read_source_file(char* source)
{
    fstream fs;

    fs.open(source, ios::in);
    if(!fs)
    {
        cerr << "Can't open file\"" << source << "\"\n";
        cerr << "Please give network node file, program aborting...\n";
        exit(1);
    }

    int node_a, max_num_dest;
    double node_b;
    while(fs >> node_a >> node_b >> max_num_dest)
    {
        node_a--;
        source_matrix[node_a] = node_b;
        num_dest_matrix[node_a] = max_num_dest;
    }

    fs.close();

}

void phy_graph::read_traffic_file(char* traffic)
{
    fstream fs;

    fs.open(traffic, ios::in);
    if(!fs)
    {
        cerr << "Can't open file\"" << traffic << "\"\n";
        cerr << "Please give network node file, program aborting...\n";
        exit(1);
    }

    int node_a, node_b;
    double traffic_probility;
    while(fs >> node_a >> node_b >> traffic_probility)
    {
        node_a--;
        node_b--;
        traffic_matrix[node_a][node_b] = traffic_probility;
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
