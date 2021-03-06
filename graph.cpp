#include "graph.hpp"

using namespace std;

int Phy_graph::modlev(int dis){
    if(dis <= transmission_distance_16QAM)
    {
        return 4;
    }
    if(dis <= transmission_distance_8QAM)
    {
        return 3;
    }
    if(dis <= transmission_distance_QPSK)
    {
        return 2;
    }
    return -1;
}

int Phy_graph::get_reach(vector<int> path)
{
    int from;
    int to;
    int dis = 0;
    for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
    {
        from = path[node_i];
        to = path[node_i + 1];
        dis += get_link(from, to).distance;
    }
    return modlev(dis);
}

Phy_graph::Phy_graph(Graph_info &g_info)
{
    this->transmission_distance_QPSK = g_info.transmission_distance_QPSK;
    this->transmission_distance_8QAM = g_info.transmission_distance_8QAM;
    this->transmission_distance_16QAM = g_info.transmission_distance_16QAM;
    read_network_file(g_info.graph_file, g_info.num_slots);
    // assign transceivers for each node
    assign_transceivers(g_info.num_OTDM_transceiver, g_info.num_OFDM_transceiver, g_info.transceiver_connection_limit);
    find_candidate_path();
}

Phy_graph::~Phy_graph()
{

}

void Phy_graph::assign_transceivers(int num_OTDM_transceiver, int num_OFDM_transceiver, int transceiver_connection_limit)
{
    for(auto &node : node_list)
    {
        node.transmitter.resize(num_OTDM_transceiver * node.degree);
        node.num_available_transmitter = num_OTDM_transceiver * node.degree;

        node.receiver.resize(num_OTDM_transceiver * node.degree);
        node.num_available_receiver = num_OTDM_transceiver * node.degree;

        node.OFDMtransmitter.resize(num_OFDM_transceiver * node.degree, OFDMTransceiver(transceiver_connection_limit));
        node.num_available_OFDM_transmitter = num_OFDM_transceiver * node.degree;

        node.OFDMreceiver.resize(num_OFDM_transceiver * node.degree, OFDMTransceiver(transceiver_connection_limit));
        node.num_available_OFDM_receiver = num_OFDM_transceiver * node.degree;
    }
}

void Phy_graph::find_candidate_path()
{
    for(unsigned int source = 0; source < node_list.size(); source++)
    {
        for(unsigned int destination = 0; destination < node_list.size(); destination++)
        {
            BFS_find_path(source, destination);
        }
    }
}

// only find the path with minimum hops
void Phy_graph::BFS_find_path(int source, int destination)
{
    int num_nodes = node_list.size();
    vector<Parents> parents;
    parents.resize(num_nodes);
    vector<int> distance(num_nodes, 1 << 30);
    vector<bool> visited(num_nodes, false);
    queue<int> Q;

    // BFS
    Q.push(source);
    distance[source] = 0;

    while (!Q.empty())
    {
        int v = Q.front();
        Q.pop();

        if(visited[v])
        {
            continue;
        }

        for(auto &w : get_node(v).neighbor)
        {
            if(distance[w] > distance[v])
            {
                distance[w] = distance[v] + 1;
                parents[w].push_back(v);
                Q.push(w);
            }
        }

        visited[v] = true;

    }

    CandidatePath path;
    DFS_back_trace(destination, parents, get_path_list(source, destination), path);
}

void Phy_graph::DFS_back_trace(int current_node, vector<Parents>& parents, list<CandidatePath>& path_set, CandidatePath& path)
{
    path.path.push_back(current_node);

    if(parents[current_node].empty())
    {
        CandidatePath new_path = path;

        reverse(new_path.path.begin(), new_path.path.end());

        int modulation = get_reach(new_path.path);

        if(modulation != -1)
        {
            new_path.modulation_level = modulation;
            path_set.push_back(new_path);
        }
    }

    for(auto &parent : parents[current_node])
    {
        DFS_back_trace(parent, parents, path_set, path);
    }

    path.path.pop_back();
}


Phy_node& Phy_graph::get_node(int id)
{
    return node_list[id];
}

Phy_link& Phy_graph::get_link(int source, int destination)
{
    return link_list[make_pair(source,destination)];
}

list<CandidatePath>& Phy_graph::get_path_list(int source, int destination)
{
    return path_list[make_pair(source, destination)];
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
        new_link.slot.resize(num_slots, -1);

        link_list[make_pair(node_a,node_b)] = new_link;

        new_link.source = node_b;
        new_link.destination = node_a;

        link_list[make_pair(node_b,node_a)] = new_link;
    }

    fs.close();
}


Phy_node::Phy_node()
{
    degree = 0;
    num_available_transmitter = 0;
    num_available_OFDM_transmitter = 0;
    num_available_receiver = 0;
    num_available_OFDM_receiver = 0;
}

Phy_node::~Phy_node()
{

}

Phy_link::Phy_link()
{
    source = 0;
    destination = 0;
    distance = 0;
}

Phy_link::~Phy_link()
{

}

Transceiver::Transceiver()
{
    this->spectrum.slot_st = -1;
    this->spectrum.slot_ed = -1;
}

Transceiver::~Transceiver()
{

}

OFDMTransceiver::OFDMTransceiver(int transceiver_connection_limit)
{
    this->sub_transceiver.resize(transceiver_connection_limit);
    this->num_available_sub_transceiver = transceiver_connection_limit;
    this->in_used = false;
}

OFDMTransceiver::~OFDMTransceiver()
{

}

CandidatePath::CandidatePath()
{

}

CandidatePath::~CandidatePath()
{

}
