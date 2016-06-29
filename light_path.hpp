#ifndef LIGHT_PATH_H
#define LIGHT_PATH_H


#include "spectrum.hpp"
#include "graph.hpp"
#include <iostream>
#include <vector>
#include <list>
#include <set>

typedef std::vector<int> Path;

class Aux_link;

class LightPath
{
public:
    enum LightPath_type{
        new_OTDM,
        new_OFDM,
        groomed_OFDM,
        electrical
    };
    LightPath_type type;
    std::set<int> requests;          // store request_id
    std::list<Aux_link*> aux_link_list;
    int modulation_level;
    int available_bitrate;
    double weight;
    Spectrum spectrum;
    Path p_path;                     // store phy_node_id
    std::vector<int> a_nodes;        // store aux_node_id

    // transmitter_index[i] == -1 -> transmitter not used on Phy_node(p_path[i])
    // transmitter_index[i] == id -> transmitter[id] used on Phy_node(p_path[i])
    std::vector<int> transmitter_index;
    std::vector<int> receiver_index;

    LightPath();
    virtual ~LightPath();
};

#endif /* LIGHT_PATH_H */
