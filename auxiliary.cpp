#include "auxiliary.hpp"

using namespace std;

Aux_node::Aux_node(int phy_id, Aux_node_type type)
{
    this -> phy_id = phy_id;
    this -> type = type;
    first_in = NULL;
    last_in = NULL;
    first_out = NULL;
    last_out = NULL;
}

Aux_node::~Aux_node()
{
    Aux_link *current;
    Aux_link *next;

    // remove all the incoming links
    current = this -> first_in;
    while(current != NULL)
    {
        next = current -> next_same_to;
        delete current;
        current = next;
    }

    // remove all the outgoing links
    current = this -> first_out;
    while(current != NULL)
    {
        next = current -> next_same_from;
        delete current;
        current = next;
    }
}

Aux_link::Aux_link(Aux_node* from, Aux_node* to, double weight, Aux_link_type type)
{
    this -> light_path = NULL;
    this -> from = from;
    this -> to = to;
    this -> weight = weight;
    this -> type = type;

    // same_from list
    Aux_link *from_last_out = from -> last_out;
    if(from_last_out == NULL) // list is empty;
    {
        this -> prev_same_from = NULL;
        this -> next_same_from = NULL;
        from -> first_out = this;
        from -> last_out = this;
    }
    else
    {
        this -> prev_same_from = from_last_out;
        this -> next_same_from = NULL;
        from_last_out -> next_same_from = this;
        from -> last_out = this;
    }

    // same_to list
    Aux_link *to_last_in = to -> last_in;
    if(to_last_in == NULL) // list is empty;
    {
        this -> prev_same_to = NULL;
        this -> next_same_to = NULL;
        to -> first_in = this;
        to -> last_in = this;
    }
    else
    {
        this -> prev_same_to = to_last_in;
        this -> next_same_to = NULL;
        to_last_in -> next_same_to = this;
        to -> last_in = this;
    }
}

Aux_link::~Aux_link()
{
    Aux_link *prev;
    Aux_link *next;

    // maintain same_from list
    prev = this -> prev_same_from;
    next = this -> next_same_from;

    if(prev == NULL && next == NULL) // link is the only element in same_from list
    {
        from -> first_out = NULL;
        from -> last_out = NULL;
    }
    else if(prev == NULL) // link is the first in same_from list
    {
        from -> first_out = next;
        next -> prev_same_from = NULL;
    }
    else if(next == NULL) // link is the last in same_from list
    {
        from -> last_out = prev;
        prev -> next_same_from = NULL;
    }
    else
    {
        prev -> next_same_from = next;
        next -> prev_same_from = prev;
    }

    // maintain same_to list
    prev = this -> prev_same_to;
    next = this -> next_same_to;

    if(prev == NULL && next == NULL) // link is the only element in same_to list
    {
        to -> first_in = NULL;
        to -> last_in = NULL;
    }
    else if(prev == NULL) // link is the first in same_to list
    {
        to -> first_in = next;
        next -> prev_same_to = NULL;
    }
    else if(next == NULL) // link is the last in same_to list
    {
        to -> last_in = prev;
        prev -> next_same_to = NULL;
    }
    else
    {
        prev -> next_same_to = next;
        next -> prev_same_to = prev;
    }

}

Aux_graph::Aux_graph(Auxiliary_info& a_info)
{
    // reserve space for all possible auxiliary nodes
    adding_node_list.reserve(a_info.num_nodes);
    dropping_node_list.reserve(a_info.num_nodes);
    OFDM_WOB_virtual_transmitting_node_list.reserve(a_info.num_nodes);
    OFDM_WOB_virtual_receiving_node_list.reserve(a_info.num_nodes);
    OFDM_WB_virtual_transmitting_node_list.reserve(a_info.num_nodes);
    OFDM_WB_virtual_receiving_node_list.reserve(a_info.num_nodes);
    OFDM_virtual_transmitting_node_list.reserve(a_info.num_nodes);
    OFDM_virtual_receiving_node_list.reserve(a_info.num_nodes);
    OTDM_virtual_transmitting_node_list.reserve(a_info.num_nodes);
    OTDM_virtual_receiving_node_list.reserve(a_info.num_nodes);

    Aux_node* a_node;   // adding node
    Aux_node* d_node;   // dropping node
    Aux_node* OFDM_WOB_v_t_node; // OFDM_WOB virtual transmitting node
    Aux_node* OFDM_WOB_v_r_node; // OFDM_WOB virtual receiving node
    Aux_node* OFDM_WB_v_t_node; // OFDM_WB virtual transmitting node
    Aux_node* OFDM_WB_v_r_node; // OFDM_WB virtual receiving node
    Aux_node* OFDM_v_t_node; // OFDM virtual transmitting node
    Aux_node* OFDM_v_r_node; // OFDM virtual receiving node
    Aux_node* OTDM_v_t_node; // OTDM virtual transmitting node
    Aux_node* OTDM_v_r_node; // OTDM virtual receiving node

    for(int phy_node_i = 0; phy_node_i < a_info.num_nodes; phy_node_i++)
    {
        a_node = create_aux_node(phy_node_i, Aux_node::adding_node);
        adding_node_list.push_back(a_node);
        d_node = create_aux_node(phy_node_i, Aux_node::dropping_node);
        dropping_node_list.push_back(d_node);

        OFDM_WOB_v_t_node = create_aux_node(phy_node_i, Aux_node::virtual_transmitting_node);
        OFDM_WOB_virtual_transmitting_node_list.push_back(OFDM_WOB_v_t_node);
        OFDM_WOB_v_r_node = create_aux_node(phy_node_i, Aux_node::virtual_receiving_node);
        OFDM_WOB_virtual_receiving_node_list.push_back(OFDM_WOB_v_r_node);

        OFDM_WB_v_t_node = create_aux_node(phy_node_i, Aux_node::virtual_transmitting_node);
        OFDM_WB_virtual_transmitting_node_list.push_back(OFDM_WB_v_t_node);
        OFDM_WB_v_r_node = create_aux_node(phy_node_i, Aux_node::virtual_receiving_node);
        OFDM_WB_virtual_receiving_node_list.push_back(OFDM_WB_v_r_node);

        OFDM_v_t_node = create_aux_node(phy_node_i, Aux_node::virtual_transmitting_node);
        OFDM_virtual_transmitting_node_list.push_back(OFDM_v_t_node);
        OFDM_v_r_node = create_aux_node(phy_node_i, Aux_node::virtual_receiving_node);
        OFDM_virtual_receiving_node_list.push_back(OFDM_v_r_node);

        OTDM_v_t_node = create_aux_node(phy_node_i, Aux_node::virtual_transmitting_node);
        OTDM_virtual_transmitting_node_list.push_back(OTDM_v_t_node);
        OTDM_v_r_node = create_aux_node(phy_node_i, Aux_node::virtual_receiving_node);
        OTDM_virtual_receiving_node_list.push_back(OTDM_v_r_node);


        create_aux_link(d_node, a_node, a_info.OEO_weight, Aux_link::grooming_link);

        create_aux_link(a_node, OFDM_WOB_v_t_node, a_info.used_OFDM_transceiver_weight, Aux_link::adding_link);
        create_aux_link(OFDM_WOB_v_r_node, d_node, a_info.used_OFDM_transceiver_weight, Aux_link::dropping_link);

        create_aux_link(a_node, OFDM_WB_v_t_node, a_info.used_OFDM_transceiver_weight, Aux_link::adding_link);
        create_aux_link(OFDM_WB_v_r_node, d_node, a_info.OFDM_transceiver_weight, Aux_link::virtual_dropping_link);

        create_aux_link(a_node, OFDM_v_t_node, a_info.OFDM_transceiver_weight, Aux_link::virtual_adding_link);
        create_aux_link(OFDM_v_r_node, d_node, a_info.OFDM_transceiver_weight, Aux_link::virtual_dropping_link);

        create_aux_link(a_node, OTDM_v_t_node, a_info.transceiver_weight, Aux_link::virtual_adding_link);
        create_aux_link(OTDM_v_r_node, d_node, a_info.transceiver_weight, Aux_link::virtual_dropping_link);
    }

}

Aux_graph::~Aux_graph()
{
    while(!adding_node_list.empty())
    {
        delete adding_node_list.back();
        adding_node_list.pop_back();
    }
    while(!dropping_node_list.empty())
    {
        delete dropping_node_list.back();
        dropping_node_list.pop_back();
    }

    while(!OFDM_WOB_virtual_transmitting_node_list.empty())
    {
        delete OFDM_WOB_virtual_transmitting_node_list.back();
        OFDM_WOB_virtual_transmitting_node_list.pop_back();
    }
    while(!OFDM_WOB_virtual_receiving_node_list.empty())
    {
        delete OFDM_WOB_virtual_receiving_node_list.back();
        OFDM_WOB_virtual_receiving_node_list.pop_back();
    }

    while(!OFDM_WB_virtual_transmitting_node_list.empty())
    {
        delete OFDM_WB_virtual_transmitting_node_list.back();
        OFDM_WB_virtual_transmitting_node_list.pop_back();
    }
    while(!OFDM_WB_virtual_receiving_node_list.empty())
    {
        delete OFDM_WB_virtual_receiving_node_list.back();
        OFDM_WB_virtual_receiving_node_list.pop_back();
    }

    while(!OFDM_virtual_transmitting_node_list.empty())
    {
        delete OFDM_virtual_transmitting_node_list.back();
        OFDM_virtual_transmitting_node_list.pop_back();
    }
    while(!OFDM_virtual_receiving_node_list.empty())
    {
        delete OFDM_virtual_receiving_node_list.back();
        OFDM_virtual_receiving_node_list.pop_back();
    }

    while(!OTDM_virtual_transmitting_node_list.empty())
    {
        delete OTDM_virtual_transmitting_node_list.back();
        OTDM_virtual_transmitting_node_list.pop_back();
    }
    while(!OTDM_virtual_receiving_node_list.empty())
    {
        delete OTDM_virtual_receiving_node_list.back();
        OTDM_virtual_receiving_node_list.pop_back();
    }
}

Aux_node* Aux_graph::create_aux_node(int phy_id, Aux_node::Aux_node_type type)
{
    Aux_node* new_node = new Aux_node(phy_id, type);
    return new_node;
}

Aux_link* Aux_graph::create_aux_link(Aux_node* from, Aux_node* to, double weight, Aux_link::Aux_link_type type)
{
    Aux_link* new_link = new Aux_link(from, to, weight, type);
    return new_link;
}

Aux_node* Aux_graph::get_adding_node(int phy_id)
{
    return adding_node_list[phy_id];
}

Aux_node* Aux_graph::get_dropping_node(int phy_id)
{
    return dropping_node_list[phy_id];
}

Aux_node* Aux_graph::get_OFDM_WOB_virtual_transmitting_node(int phy_id)
{
    return OFDM_WOB_virtual_transmitting_node_list[phy_id];
}

Aux_node* Aux_graph::get_OFDM_WOB_virtual_receiving_node(int phy_id)
{
    return OFDM_WOB_virtual_receiving_node_list[phy_id];
}

Aux_node* Aux_graph::get_OFDM_WB_virtual_transmitting_node(int phy_id)
{
    return OFDM_WB_virtual_transmitting_node_list[phy_id];
}

Aux_node* Aux_graph::get_OFDM_WB_virtual_receiving_node(int phy_id)
{
    return OFDM_WB_virtual_receiving_node_list[phy_id];
}

Aux_node* Aux_graph::get_OFDM_virtual_transmitting_node(int phy_id)
{
    return OFDM_virtual_transmitting_node_list[phy_id];
}

Aux_node* Aux_graph::get_OFDM_virtual_receiving_node(int phy_id)
{
    return OFDM_virtual_receiving_node_list[phy_id];
}

Aux_node* Aux_graph::get_OTDM_virtual_transmitting_node(int phy_id)
{
    return OTDM_virtual_transmitting_node_list[phy_id];
}

Aux_node* Aux_graph::get_OTDM_virtual_receiving_node(int phy_id)
{
    return OTDM_virtual_receiving_node_list[phy_id];
}
