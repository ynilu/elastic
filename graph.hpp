#ifndef GRAPH_H
#define GRAPH_H

class LightPath;

#include "light_path.hpp"
#include "spectrum.hpp"

#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <queue>
#include <utility>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <map>

struct Graph_info
{
    char* graph_file;
    int num_slots;
    int num_OTDM_transceiver;
    int num_OFDM_transceiver;
    int slot_capacity;
    int transceiver_slot_limit;
    int transceiver_connection_limit;
    int transmission_distance_16QAM;
    int transmission_distance_8QAM;
    int transmission_distance_QPSK;
};

class Transceiver
{
public:
    Spectrum spectrum;

    Transceiver();
    virtual ~Transceiver();
};

class OFDMTransceiver // OFDM transceiver
{
public:

    bool in_used;
    int num_available_sub_transceiver;
    std::list<LightPath*> light_path;
    std::vector<Transceiver> sub_transceiver;

    Spectrum spectrum;

    OFDMTransceiver(int transceiver_connection_limit);
    virtual ~OFDMTransceiver();
};


class Phy_node // physical node
{
public:
    int degree;
    std::vector<int> neighbor;
    int num_available_transmitter;
    int num_available_OFDM_transmitter;
    int num_available_receiver;
    int num_available_OFDM_receiver;
    std::vector<Transceiver> transmitter;
    std::vector<Transceiver> receiver;
    std::vector<OFDMTransceiver> OFDMtransmitter;
    std::vector<OFDMTransceiver> OFDMreceiver;
    Phy_node();
    virtual ~Phy_node();
};

class Phy_link // physical link
{
public:
    int source;
    int destination;
    int distance;
    std::vector<int> slot;   // -1 -> free; 0 -> guardband; 1 -> occupied
    Phy_link();
    virtual ~Phy_link();
};

typedef std::vector<int> Path;

class CandidatePath
{
public:
    Path path;
    int modulation_level;    // 2 -> QPSK; 3 -> 8QAM; 4 -> 16QAM
    CandidatePath();
    virtual ~CandidatePath();
};


typedef std::pair<int, int> Node_pair;
typedef std::vector<int> Parents;
typedef std::map< Node_pair, std::list<CandidatePath> > Candidate_path_list;
typedef std::vector<Phy_node> Phy_node_list;
typedef std::map< Node_pair, Phy_link > Phy_link_list;

class Phy_graph // physical graph
{
public:
    Phy_node_list node_list;
    Phy_link_list link_list;
    Candidate_path_list path_list;
    int transmission_distance_16QAM;
    int transmission_distance_8QAM;
    int transmission_distance_QPSK;

    void read_network_file(char* graph_file, int num_slots);

    void assign_transceivers(int num_OTDM_transceiver, int num_OFDM_transceiver, int transceiver_connection_limit);
    void find_candidate_path();
    void BFS_find_path(int source, int destination);
    void DFS_back_trace(int current_node, std::vector<Parents>& parents, std::list<CandidatePath>& path_set, CandidatePath& path);
    int get_reach(std::vector<int> path);
    int modlev(int dis);

    Phy_node& get_node(int id);
    Phy_link& get_link(int source, int destination);


    std::list<CandidatePath>& get_path_list(int source, int destination);

    Phy_graph(Graph_info &g_info);
    virtual ~Phy_graph();
};

#endif /* GRAPH_H */




