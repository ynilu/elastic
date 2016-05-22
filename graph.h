#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <map>
#include <utility>
#include <iostream>
#include <fstream>

struct graph_info
{
    char* graph;
    char* source;
    char* traffic;
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
    std::vector<int> neighbour;
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
        std::vector< std::vector<double> > traffic_matrix;
        std::vector<double> source_matrix;
        std::vector<int> num_dest_matrix;

    phy_graph(graph_info &info);

    void read_network_file(char* graph, int num_of_slots);
    void read_source_file(char* source);
    void read_traffic_file(char* traffic);

    phy_node& get_node(int id);
    phy_link& get_link(int source, int destination);

    virtual ~phy_graph();
};

#endif
