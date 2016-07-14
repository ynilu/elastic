#ifndef AUXILIARY_H
#define AUXILIARY_H

#include "light_path.hpp"
#include <iostream>
#include <vector>

class Aux_link;

struct Auxiliary_info
{
    int num_slots;
    int num_nodes;
    double transceiver_weight;
    double used_transceiver_weight;
    double OFDM_transceiver_weight;
    double used_OFDM_transceiver_weight;
    double OEO_weight;
};

class Aux_node
{
public:
    enum Aux_node_type{
        adding_node,
        dropping_node,
        transmitting_node,
        receiving_node,
        virtual_transmitting_node,
        virtual_receiving_node
    };

    Aux_node_type type;
    int phy_id; // the physical node which this belongs to

    Aux_link *first_in, *last_in, *first_out, *last_out;

    Aux_node(int phy_id, Aux_node_type type);
    virtual ~Aux_node();
};

class Aux_link
{
public:
    enum Aux_link_type{
        candidate_link,
        spectrum_link,
        grooming_link,
        adding_link,
        dropping_link,
        pass_through_link,
        virtual_adding_link,
        virtual_dropping_link
    };

    LightPath* light_path; // the light path which this link represents

    Aux_link_type type;
    double weight;

    Aux_node *from, *to;
    Aux_link *prev_same_from, *next_same_from, *prev_same_to, *next_same_to;

    Aux_link(Aux_node* from, Aux_node* to, double weight, Aux_link_type type);
    virtual ~Aux_link();
};

class Aux_graph
{
public:
    std::vector<Aux_node*> adding_node_list;
    std::vector<Aux_node*> dropping_node_list;
    std::vector<Aux_node*> electrical_virtual_transmitting_node_list;
    std::vector<Aux_node*> electrical_virtual_receiving_node_list;
    std::vector<Aux_node*> groomed_OFDM_virtual_transmitting_node_list;
    std::vector<Aux_node*> groomed_OFDM_virtual_receiving_node_list;
    std::vector<Aux_node*> new_OFDM_virtual_transmitting_node_list;
    std::vector<Aux_node*> new_OFDM_virtual_receiving_node_list;
    std::vector<Aux_node*> new_OTDM_virtual_transmitting_node_list;
    std::vector<Aux_node*> new_OTDM_virtual_receiving_node_list;

    Aux_node* get_adding_node(int phy_id);
    Aux_node* get_dropping_node(int phy_id);

    Aux_node* get_electrical_virtual_transmitting_node(int phy_id);
    Aux_node* get_electrical_virtual_receiving_node(int phy_id);
    Aux_node* get_groomed_OFDM_virtual_transmitting_node(int phy_id);
    Aux_node* get_groomed_OFDM_virtual_receiving_node(int phy_id);
    Aux_node* get_new_OFDM_virtual_transmitting_node(int phy_id);
    Aux_node* get_new_OFDM_virtual_receiving_node(int phy_id);
    Aux_node* get_new_OTDM_virtual_transmitting_node(int phy_id);
    Aux_node* get_new_OTDM_virtual_receiving_node(int phy_id);

    Aux_node* create_aux_node(int phy_id, Aux_node::Aux_node_type type);
    Aux_link* create_aux_link(Aux_node* from, Aux_node* to, double weight, Aux_link::Aux_link_type type);

    Aux_graph(Auxiliary_info& a_info);
    virtual ~Aux_graph();
};

#endif /* AUXILIARY_H */
