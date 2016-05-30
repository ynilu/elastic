#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <map>
#include <set>
#include <queue>
#include <utility>
#include <iostream>
#include <fstream>

struct Graph_info
{
    char* graph_file;
    int slot_limit;
    int connection_limit;
    int num_slots;
};


struct Spectrum_result
{
    double weight;
    int slot_st;  // start number of occupied slots
    int slot_ed;  // end number of occupied slots
};

class Receiver
{
public:
    int slot_st;  // start number of occupied slots
    int slot_ed;  // end number of occupied slots

    Receiver();
    virtual ~Receiver();
};


class Transmitter
{
public:
    int slot_st;  // start number of occupied slots
    int slot_ed;  // end number of occupied slots

    Transmitter();
    virtual ~Transmitter();

};


class Phy_node // physical node
{
public:
    int degree;
    std::vector<int> neighbor;
    Phy_node();
    virtual ~Phy_node();
};


class Phy_link // physical link
{
public:
    int source;
    int destination;
    int distance;
    std::vector<int> slot;
    Phy_link();
    virtual ~Phy_link();
};


typedef std::pair Node_pair;
typedef std::vector<int> Path;
typedef std::map< Node_pair, Path > Path_list;
typedef std::vector<Phy_node> Phy_node_list;
typedef std::map< Node_pair, Phy_link > Phy_link_list;

class Phy_graph // physical graph
{
public:
    Phy_node_list node_list;
    Phy_link_list link_list;

    Phy_graph(Graph_info &info);

    void read_network_file(char* graph_file, int num_slots);

    void find_candidate_path();

    Spectrum_result find_best_spectrum(Path& path, int require_slots);
    bool spectrum_available(Path& path, int slot_st, int slot_ed);
    double get_spectrum_weight(Path& path, int slot_st, int slot_ed);

    Phy_node& get_node(int id);
    Phy_link& get_link(int source, int destination);

    virtual ~Phy_graph();
};

#endif /* GRAPH_H */
