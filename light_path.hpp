#ifndef LIGHT_PATH_H
#define LIGHT_PATH_H

#include "auxiliary.hpp"
#include "graph.hpp"
#include <iostream>
#include <list>
#include <set>

class LightPath
{
public:
    std::set<int> requests;          // store request_id
    std::list<Aux_link> edges;
    int bandwidth;                   // used_bandwidth
    Spectrum spectrum;
    std::list<int> p_nodes;          // store phy_node_id
    std::list<int> a_nodes;          // store aux_node_id

    LightPath();
    virtual ~LightPath();
};

#endif /* LIGHT_PATH_H */
