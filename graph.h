#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <map>
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


class Phy_graph // physical graph
{
public:
    std::vector<Phy_node> node;
    std::map< std::pair< int, int >, Phy_link > link;

    Phy_graph(Graph_info &info);

    void read_network_file(char* graph_file, int num_slots);

    Phy_node& get_node(int id);
    Phy_link& get_link(int source, int destination);

    virtual ~Phy_graph();
};

#endif /* GRAPH_H */
