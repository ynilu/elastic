#ifndef LIGHT_PATH_H
#define LIGHT_PATH_H

#include "graph.hpp"
#include <iostream>
#include <vector>
#include <list>
#include <set>

class Aux_link;

class LightPath
{
public:
    enum LightPath_type{
        new_OTDM,
        new_OFDM,
        groomed_OFDM
    };
    LightPath_type type;
    std::set<int> requests;          // store request_id
    std::list<Aux_link*> aux_link_list;
    int modulation_level;
    double used_bandwidth;           // used_bandwidth
    double weight;
    Spectrum spectrum;
    Path p_path;                     // store phy_node_id
    std::vector<int> a_nodes;        // store aux_node_id

    LightPath();
    virtual ~LightPath();
};

#endif /* LIGHT_PATH_H */
