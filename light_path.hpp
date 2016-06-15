#ifndef LIGHT_PATH_H
#define LIGHT_PATH_H

#include "auxiliary.hpp"
#include "graph.hpp"
#include <iostream>
#incldue <vector>
#include <list>
#include <set>

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
    int bandwidth;                   // used_bandwidth
    double weight;
    Spectrum spectrum;
    Path p_path;                     // store phy_node_id
    std::vector<int> a_nodes;        // store aux_node_id


    LightPath();
    virtual ~LightPath();
};

#endif /* LIGHT_PATH_H */
