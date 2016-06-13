#include "graph.h"
#include "traffic.h"
#include "auxiliary.h"
#include "light_path.h"
#include <iostream>
#include <cmath>

using namespace std;

float unicast_percentage = 1.0;
int num_requests = 10;
int num_slots = 360;
int num_nodes;
int bandwidth_max = 50;
int bandwidth_min = 1;
int traffic_lambda = 1000;
int traffic_mu = 1;
int num_OTDM_transceiver = 400;
int num_OFDM_transceiver = 400;
int slot_capacity = 12;
int transceiver_slot_limit = 20;
int transceiver_connection_limit = 20;

//random_variables
long long aTime_seed = 1491701989;
long long hTime_seed = 82684867;
long long s_seed = 176028846;
long long d_seed = 1244571654;
long long numD_seed = 2029305454;
long long b_seed = 453424686;

//result variable
int accepted_requests = 0;
int blocked_requests = 0;
int bandwidth_blocked = 0;
int trail_usage_count = 0;
int trail_usage_count_back = 0;

double total_bandwidth_utilization=0;
double used_bandwidth_utilization=0;
double total_bandwidth_utilization_back=0;
double used_bandwidth_utilization_back=0;
double transceiver_weigth = 0.3;
double grooming_weight = 0.3;
double wavelength_edge_weight = 1000;
double used_wavelength_edge_weight = 999.9997;
double busy_edge_weight = 0;
double extra_used_transmitter=0;
double extra_used_transmitter_back=0;
double extra_used_receiver=0;
double extra_used_receiver_back=0;

void construct_candidate_path(Event& event, Phy_graph& p_graph);
LightPath* get_best_new_OTDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph);
LightPath* get_best_new_OFDM_light_path();
LightPath* get_best_groomed_OFDM_light_path();

int main(int argc, char *argv[])
{
    Graph_info g_info;
    g_info.graph_file = (char*) "USnet.txt";
    g_info.num_slots = num_slots;
    g_info.num_OTDM_transceiver = num_OTDM_transceiver;
    g_info.num_OFDM_transceiver = num_OFDM_transceiver;
    g_info.slot_capacity = slot_capacity;
    g_info.transceiver_slot_limit = transceiver_slot_limit;
    g_info.transceiver_connection_limit = transceiver_connection_limit;

    Phy_graph p_graph(g_info);

    cout << "num_nodes : " << p_graph.node_list.size() << "\n";
    num_nodes = p_graph.node_list.size();

    Traffic_info t_info;
    t_info.source_file = (char*) "USnet_source.txt";
    t_info.traffic_file = (char*) "USnet_traffic.txt";
    t_info.num_nodes = p_graph.node_list.size();
    t_info.num_requests = num_requests;
    t_info.bandwidth_max = bandwidth_max;
    t_info.bandwidth_min = bandwidth_min;
    t_info.traffic_lambda = traffic_lambda;
    t_info.traffic_mu = traffic_mu;
    t_info.unicast_percentage = unicast_percentage;

    t_info.aTime_seed = aTime_seed;
    t_info.hTime_seed = hTime_seed;
    t_info.s_seed = s_seed;
    t_info.d_seed = d_seed;
    t_info.numD_seed = numD_seed;
    t_info.b_seed = b_seed;

    Traffic traffic (t_info);

    // Phy_node& a = p_graph.get_node(0);
    // Phy_link& b = p_graph.get_link(0,1);

    // cout << a.degree << '\n';
    // cout << b.source << '\n';

    Aux_graph a_graph(num_nodes, num_slots);

    // for(int source = 0; source < p_graph.node_list.size(); source++)
    // {
    //     for(int destination = 0; destination < p_graph.node_list.size(); destination++)
    //     {
    //         list<Path>& a = p_graph.get_path_list(source, destination);
    //         for(auto &path : a)
    //         {
    //             for(auto &node : path.path)
    //             {
    //                 cout << node << " ";
    //             }
    //             cout << '\n';
    //         }
    //     }
    // }

    cout << p_graph.get_node(0).OFDMtransmitter[0].sub_transmitter.size() << '\n';
    cout << p_graph.get_node(0).OFDMtransmitter.size() << '\n';

    while( !traffic.empty() )
    {
        Event event = traffic.next_event();
        if(event.type == Event::arrival)
        {
            // cout << "arrival :\nid = " << event.request_id << "\nsource = " << event.source << "\ndest = " << *event.destination.begin() << "\nbandwidth = " << event.bandwidth << "\narrivaltime = " << event.arrival_time << "\n\n";
            construct_candidate_path(event, p_graph);
            // construct_exist_path();
        }
        else // if(event.type == Event::departure)
        {
            // cout << "departure :\nid = " << event.request_id << "\nsource = " << event.source << "\ndest = " << *event.destination.begin() << "\nbandwidth = " << event.bandwidth << "\narrivaltime = " << event.arrival_time << "\n\n";
        }
    }


    return 0;
}

void construct_candidate_path(Event& event, Phy_graph& p_graph)
{
    for(int source = 0; source < num_nodes; source++)
    {
        for(int destination = 0; destination < num_nodes; destination++)
        {
            LightPath *new_OTDM_lp = get_best_new_OTDM_light_path(source, destination, event, p_graph);
            // LightPath *new_OFDM_lp = get_best_new_OFDM_light_path();
            // LightPath *groomed_OFDM_lp = get_best_groomed_OFDM_light_path();
        }
    }

    // LightPath *new_OTDM_lp = get_best_new_OTDM_light_path();
    // LightPath *new_OFDM_lp = get_best_new_OFDM_light_path();
    // LightPath *groomed_OFDM_lp = get_best_groomed_OFDM_light_path();


    // delete new_OTDM_lp;
    // delete new_OFDM_lp;
    // delete groomed_OFDM_lp;
}

bool spectrum_available(int from, int to, int slot_st, int slot_ed, Phy_graph& p_graph)
{
    for(int i = slot_st; i <= slot_ed; i++)
    {
        if(p_graph.get_link(from, to).slot[i] != -1)
        {
            return false;
        }
    }
    return true;
}

bool path_spectrum_available(Path path, int slot_st, int slot_ed, Phy_graph& p_graph)
{
    for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
    {
        int from = path[node_i];
        int to = path[node_i + 1];

        if(!spectrum_available(from, to, slot_st, slot_ed, p_graph))
        {
            return false;
        }
    }
    return true;
}

Spectrum find_best_spectrum(Path path, int require_slots, Phy_graph& p_graph)
{
    int slot_st;
    int slot_ed;
    Spectrum sp;
    for(slot_st = 0; slot_st <= num_slots - require_slots; slot_st++)
    {
        slot_ed = slot_st + require_slots - 1;
        if(path_spectrum_available(path, slot_st, slot_ed, p_graph))
        {
            sp.slot_st = slot_st;
            sp.slot_ed = slot_ed;
            return sp;
        }
    }
    sp.slot_st = -1;
    sp.slot_ed = -1;
    return sp;
}

LightPath* get_best_new_OTDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
{
    Phy_node& src_node = p_graph.get_node(source);
    Phy_node& dst_node = p_graph.get_node(destination);

    if(src_node.num_available_transmitter < 1)
    {
        return NULL;
    }

    if(src_node.num_available_receiver < 1)
    {
        return NULL;
    }

    list<CandidatePath>& c_path_list = p_graph.get_path_list(source, destination);

    for(auto &c_path : c_path_list)
    {
        int require_slots = ceil(1.0 * event.bandwidth / c_path.modulation_level / slot_capacity);
        Spectrum sp = find_best_spectrum(c_path.path, require_slots, p_graph);
        if(sp.slot_st == -1)
        {
            // TODO skip this path
        }
    }


    // TODO new LightPath and return

}

LightPath* get_best_new_OFDM_light_path()
{

}

LightPath* get_best_groomed_OFDM_light_path()
{

}

