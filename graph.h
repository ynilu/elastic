#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <map>
#include <utility>
#include <iostream>
#include <fstream>

struct graph_info
{
    char* graph_file;
    int slot_limit;
    int connection_limit;
    int num_of_slots;
};


class receiver
{
public:
    int slot_st;  // start number of occupied slots
    int slot_ed;  // end number of occupied slots

    receiver();
    virtual ~receiver();
};


class transmitter
{
public:
    int slot_st;  // start number of occupied slots
    int slot_ed;  // end number of occupied slots

    transmitter();
    virtual ~transmitter();

};


class phy_node // physical node
{
public:
    int degree;
    std::vector<int> neighbor;
    phy_node();
    virtual ~phy_node();
};


class phy_link // physical link
{
public:
    int source;
    int destination;
    int distance;
    std::vector<int> slot;
    phy_link();
    virtual ~phy_link();
};


class phy_graph //physical graph
{
public:
    std::vector<phy_node> node;
    std::map< std::pair< int, int >, phy_link > link;

    phy_graph(graph_info &info);

    void read_network_file(char* graph_file, int num_of_slots);

    phy_node& get_node(int id);
    phy_link& get_link(int source, int destination);

    virtual ~phy_graph();
};

#endif
