#ifndef GRAPH_H
#define GRAPH_H

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

const double LAUNCH_OSNR = 24.5; // not decided yet
const double LAUNCH_POWER_DENSITY = 30;
const double ATTENUATION_PARA = 0.2;
const double NOISE_PENALTY_FACTOR = 1;
const double AMPLIFIER_NOISE_FIGURE = 6;
const double RESIDUAL_DISPERSION_RATIO = 1;
const double FIBER_NONLINEARITY_COEFFICIENT = 1.22;
const double VELOCITY_DISPERSION_PARA = 16;
const double TOTAL_FIBER_BANDWIDTH = 4;
const double LIGHT_FREQUENCY = 196.1e3;
// const double LIGHT_FREQUENCY = 192.1e3;
const double PLANCK = 6.625e-34;
const double PI = 3.1416;
const double QPSK_OSNR = 17; // not decided yet
const double OSNR[5] = {-1, -1, 0, 2, 4};
// const double OSNR[5] = {-1, -1, 8, 13, 16};
const int SPAN_LEN = 82;

double d_osnr(int dis, int span_num, int span_len);
int modlev(std::vector<int>dis_vec);

struct Graph_info
{
    char* graph_file;
    int num_slots;
    int num_OTDM_transceiver;
    int num_OFDM_transceiver;
    int slot_capacity;
    int transceiver_slot_limit;
    int transceiver_connection_limit;
};

class Spectrum
{
public:
    int slot_st;   // start number of occupied slots
    int slot_ed;   // end number of occupied slots
    double weight;

    Spectrum();
    virtual ~Spectrum();
};

class Transceiver
{
public:
    Spectrum spectrum;

    Transceiver();
    virtual ~Transceiver();
};

class OFDMTransmitter // OFDM transmitter
{
public:

    int num_available_sub_transmitter;
    std::vector<Transceiver> sub_transmitter;

    OFDMTransmitter(int transceiver_connection_limit);
    virtual ~OFDMTransmitter();
};

class OFDMReceiver   // OFDM receiver
{
public:

    int num_available_sub_receiver;
    std::vector<Receiver> sub_receiver;

    OFDMReceiver(int transceiver_connection_limit);
    virtual ~OFDMReceiver();
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
    std::vector<Receiver> receiver;
    std::vector<OFDMTransmitter> OFDMtransmitter;
    std::vector<OFDMReceiver> OFDMreceiver;
    Phy_node();
    virtual ~Phy_node();
};

class Phy_link // physical link
{
public:
    int source;
    int destination;
    int distance;
    int num_remaining_slots;
    std::vector<int> slot;   // -1 -> free; 0 -> guardband; others -> occupied time slots
    Phy_link();
    Phy_link(int);
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


    void read_network_file(char* graph_file, int num_slots);

    void assign_transceivers(int num_OTDM_transceiver, int num_OFDM_transceiver, int transceiver_connection_limit);
    void find_candidate_path();
    void BFS_find_path(int source, int destination);
    void DFS_back_trace(int current_node, std::vector<Parents>& parents, std::list<CandidatePath>& path_set, CandidatePath& path);
    int get_reach(std::vector<int> path);
    double d_osnr(int span_num, int span_len);
    int modlev(std::vector<int>dis_vec);

    Phy_node& get_node(int id);
    Phy_link& get_link(int source, int destination);


    std::list<CandidatePath>& get_path_list(int source, int destination);

    Phy_graph(Graph_info &info);
    virtual ~Phy_graph();
};

#endif /* GRAPH_H */
