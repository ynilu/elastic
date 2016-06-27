#include "spectrum.hpp"
#include "light_path.hpp"
#include "graph.hpp"
#include "traffic.hpp"
#include "auxiliary.hpp"

#include <cfloat>
#include <queue>
#include <unordered_map>
#include <iostream>
#include <cmath>
#include <list>
#include <climits>

using namespace std;

float unicast_percentage = 1.0;
int num_requests = 10;
int num_slots = 320;
int num_nodes;
int bandwidth_max = 50;
int bandwidth_min = 1;
int traffic_lambda = 1000;
int traffic_mu = 1;
int num_OTDM_transceiver = 400;
int num_OFDM_transceiver = 400;
double slot_capacity = 12.5;
int transceiver_slot_limit = 20;
int transceiver_connection_limit = 20;
int num_guardband_slot = 1;

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
double reserved_coefficent = 10;
double cut_coeffcient = 10;
double align_coeffcient = 10;

list<LightPath*> candidate_light_path_list;
list<LightPath*> exist_OTDM_light_path_list;
list<LightPath*> exist_OFDM_light_path_list;

typedef unordered_map<Aux_node*, double> Aux_node2Double;
typedef unordered_map<Aux_node*, Aux_link*> Aux_node2Aux_link;
typedef unordered_map<Aux_node*, bool> Aux_node2Bool;

Aux_node2Aux_link BellmanFordSP(Aux_graph& a_graph, Aux_node* s);
double get_dist(Aux_node2Double& distTo, Aux_node* node);
void relax(Aux_node* v, Aux_node2Double& distTo, Aux_node2Aux_link& edgeTo, Aux_node2Bool& onQueue, queue<Aux_node*> queue);

void construct_candidate_path(Event& event, Phy_graph& p_graph, Aux_graph& a_graph);
void build_candidate_link(Aux_graph& a_graph, LightPath* lpath);
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

    cout << p_graph.get_node(0).OFDMtransmitter[0].sub_transceiver.size() << '\n';
    cout << p_graph.get_node(0).OFDMtransmitter.size() << '\n';

    while( !traffic.empty() )
    {
        Event event = traffic.next_event();
        if(event.type == Event::arrival)
        {
            // cout << "arrival :\nid = " << event.request_id << "\nsource = " << event.source << "\ndest = " << *event.destination.begin() << "\nbandwidth = " << event.bandwidth << "\narrivaltime = " << event.arrival_time << "\n\n";
            construct_candidate_path(event, p_graph, a_graph);
            // construct_exist_path();
        }
        else // if(event.type == Event::departure)
        {
            // cout << "departure :\nid = " << event.request_id << "\nsource = " << event.source << "\ndest = " << *event.destination.begin() << "\nbandwidth = " << event.bandwidth << "\narrivaltime = " << event.arrival_time << "\n\n";
        }
    }


    return 0;
}

void construct_candidate_path(Event& event, Phy_graph& p_graph, Aux_graph& a_graph)
{
    for(int source = 0; source < num_nodes; source++)
    {
        for(int destination = 0; destination < num_nodes; destination++)
        {
            if(source == destination){
                continue;
            }
            // LightPath *new_OTDM_lp = get_best_new_OTDM_light_path(source, destination, event, p_graph);
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

void build_candidate_link(Aux_graph& a_graph, LightPath* lpath)
{
    int source = lpath->p_path.front();
    int destination = lpath->p_path.back();
    Aux_node* v_t_node = a_graph.get_virtual_transmitting_node(source);
    Aux_node* v_r_node = a_graph.get_virtual_receiving_node(destination);
    Aux_link* c_link = a_graph.create_aux_link(v_t_node, v_r_node, lpath->weight, Aux_link::candidate_link);

    c_link->light_path = lpath;              // make aux_link track light path

    lpath->aux_link_list.push_back(c_link);  // make light path track aux_link

    candidate_light_path_list.push_back(lpath);
}

int num_spectrum_available(Phy_link& link, int slot_st, int slot_ed)
{
    int num_available = 0;
    for(int i = slot_st; i <= slot_ed; i++)
    {
        if(link.slot[i] == -1)
        {
            num_available++;
        }
    }
    return num_available;
}

int spectrum_available(Phy_link& link, int slot_st, int slot_ed)
{
    int i = slot_ed;
    if(link.slot[i] != -1)
    {
        i++;
        while(link.slot[i] != -1)
        {
            i++;
        }
        return i; // index of first free slot
    }
    for(i = slot_ed - 1; i >= slot_st; i--)
    {
        if(link.slot[i] != -1)
        {
            return i + 1; // index of first free slot
        }
    }
    return -1; // specturm available
}

int path_spectrum_available(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph)
{
    for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
    {
        int from = path[node_i];
        int to = path[node_i + 1];
        Phy_link& link = p_graph.get_link(from, to);
        int next_start =  spectrum_available(link, slot_st, slot_ed);
        if(next_start > 0)
        {
            return next_start;
        }
    }
    return -1; // specturn available
}

int get_distance(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph)
{
    bool zone_clear = true;
    int distance = 0;
    int search_scope = transceiver_slot_limit - (slot_ed - slot_st + 1) + transceiver_slot_limit - 1;
    for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
    {
        int from = path[node_i];
        int to = path[node_i + 1];
        Phy_link& link = p_graph.get_link(from, to);
        for(int i = 1; i < search_scope; i++)
        {
            if(link.slot[slot_st - i] == 0)
            {
                distance += i;
                zone_clear = false;
                break;
            }
        }
        for(int i = 1; i < search_scope; i++)
        {
            if(link.slot[slot_ed + i] == 0)
            {
                distance += i;
                zone_clear = false;
                break;
            }
        }
    }
    if(zone_clear){
        return INT_MAX;
    }
    return distance;
}

int get_cut_num(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph)
{
    int num_cut = 0;
    for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
    {
        int from = path[node_i];
        int to = path[node_i + 1];
        Phy_link& link = p_graph.get_link(from, to);
        if(link.slot[slot_st - 1] == -1)
        {
            num_cut++;
        }
        if(link.slot[slot_ed + 1] == -1)
        {
            num_cut++;
        }
    }
    return num_cut;
}

int get_align_num(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph)
{
    int num_align = 0;
    for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
    {
        int from = path[node_i];
        int to = path[node_i + 1];
        for(auto &w : p_graph.get_node(from).neighbor)
        {
            if(w == to)
            {
                continue;
            }
            Phy_link& link = p_graph.get_link(w, from);
            num_align += num_spectrum_available(link, slot_st, slot_ed);
        }
        for(auto &v : p_graph.get_node(to).neighbor)
        {
            if(v == from)
            {
                continue;
            }
            Phy_link& link = p_graph.get_link(to, v);
            num_align += num_spectrum_available(link, slot_st, slot_ed);
        }
    }
    return num_align;
}

double weigh_path_spectrum(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph)
{
    double distance = get_distance(path, slot_st, slot_ed, p_graph);
    double num_cut = get_cut_num(path, slot_st, slot_ed, p_graph);
    double num_align = get_align_num(path, slot_st, slot_ed, p_graph);
    return reserved_coefficent * (1 / (distance + 1)) + cut_coeffcient * num_cut + align_coeffcient * num_align;
}

Spectrum find_best_spectrum(Path& path, int require_slots, Phy_graph& p_graph)
{
    int slot_st = 0;
    int slot_ed;
    Spectrum best_path_spectrum;
    double current_weight;
    best_path_spectrum.slot_st = -1;
    best_path_spectrum.slot_ed = -1;
    best_path_spectrum.weight = DBL_MAX;
    while(slot_st <= num_slots - require_slots)
    {
        slot_ed = slot_st + require_slots - 1;
        int next_start = path_spectrum_available(path, slot_st, slot_ed, p_graph);
        if(next_start < 0) // spectrum_available
        {
            current_weight = weigh_path_spectrum(path, slot_st, slot_ed, p_graph);
            if(current_weight < best_path_spectrum.weight)
            {
                best_path_spectrum.slot_st = slot_st;
                best_path_spectrum.slot_ed = slot_ed;
                best_path_spectrum.weight = current_weight;
            }
        }
        slot_st = next_start;
    }
    return best_path_spectrum;
}

LightPath* get_best_new_OTDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
{
    Phy_node& src_node = p_graph.get_node(source);
    Phy_node& dst_node = p_graph.get_node(destination);

    if(src_node.num_available_transmitter < 1)
    {
        return NULL;
    }

    if(dst_node.num_available_receiver < 1)
    {
        return NULL;
    }

    int modulation_level;
    int available_bitrate;
    list<CandidatePath>& c_path_list = p_graph.get_path_list(source, destination);
    Spectrum path_specturm, best_path_spectrum;

    best_path_spectrum.slot_st = -1;
    best_path_spectrum.slot_ed = -1;
    best_path_spectrum.weight = DBL_MAX;

    for(auto &c_path : c_path_list)
    {
        int require_slots = num_guardband_slot * 2 + ceil(1.0 * event.bandwidth / c_path.modulation_level / slot_capacity);
        path_specturm = find_best_spectrum(c_path.path, require_slots, p_graph);
        if(best_path_spectrum.weight > path_specturm.weight){
           best_path_spectrum = path_specturm;
           modulation_level = c_path.modulation_level;
           available_bitrate = (require_slots - num_guardband_slot * 2) * c_path.modulation_level * slot_capacity - event.bandwidth;
        }
    }
    if(best_path_spectrum.slot_st < 0){
        return NULL;
    }
    LightPath* path = new LightPath();
    path->type = LightPath::new_OTDM;
    path->modulation_level = modulation_level;
    path->available_bitrate = available_bitrate;
    path->weight = best_path_spectrum.weight;
    path->spectrum = best_path_spectrum;
    return path;
}

int get_available_transceiver(vector<OFDMTransceiver> transceivers)
{
    for(int i = 0; i < (int)transceivers.size(); i++)
    {
        if(transceivers[i].in_used == false)
        {
            transceivers[i].in_used = true;
            return i;
        }
    }
    cout << "error can not find available transceiver\n";
    return -1;
}

LightPath* get_best_new_OFDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
{
    Phy_node& src_node = p_graph.get_node(source);
    Phy_node& dst_node = p_graph.get_node(destination);

    if(src_node.num_available_OFDM_transmitter < 1)
    {
        return NULL;
    }

    if(dst_node.num_available_OFDM_receiver < 1)
    {
        return NULL;
    }

    int modulation_level;
    int available_bitrate;
    list<CandidatePath>& c_path_list = p_graph.get_path_list(source, destination);
    Spectrum path_specturm, best_path_spectrum;

    best_path_spectrum.slot_st = -1;
    best_path_spectrum.slot_ed = -1;
    best_path_spectrum.weight = DBL_MAX;

    for(auto &c_path : c_path_list)
    {
        int require_slots = num_guardband_slot * 2 + ceil(1.0 * event.bandwidth / c_path.modulation_level / slot_capacity);
        path_specturm = find_best_spectrum(c_path.path, require_slots, p_graph);
        if(best_path_spectrum.weight > path_specturm.weight){
           best_path_spectrum = path_specturm;
           modulation_level = c_path.modulation_level;
           available_bitrate = (require_slots - num_guardband_slot * 2) * c_path.modulation_level * slot_capacity - event.bandwidth;
        }
    }
    if(best_path_spectrum.slot_st < 0){
        return NULL;
    }
    LightPath* path = new LightPath();
    path->type = LightPath::new_OTDM;
    path->modulation_level = modulation_level;
    path->available_bitrate = available_bitrate;
    path->weight = best_path_spectrum.weight;
    path->spectrum = best_path_spectrum;
    path->transmitter_index.resize(path->p_path.size(), -1);
    path->receiver_index.resize(path->p_path.size(), -1);
    path->transmitter_index.front() = get_available_transceiver(src_node.OFDMtransmitter);
    path->receiver_index.back() = get_available_transceiver(src_node.OFDMreceiver);
    src_node.OFDMtransmitter[path->transmitter_index.front()].spectrum = best_path_spectrum;
    dst_node.OFDMreceiver[path->receiver_index.back()].spectrum = best_path_spectrum;
    return path;
}

LightPath* get_best_electrical_groomed_OFDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
{
    Phy_node& src_node = p_graph.get_node(source);
    Phy_node& dst_node = p_graph.get_node(destination);

    Spectrum path_specturm, best_path_spectrum;
    double current_weight;
    LightPath* best_existing_lightpath;

    best_path_spectrum.slot_st = -1;
    best_path_spectrum.slot_ed = -1;
    best_path_spectrum.weight = DBL_MAX;
    best_existing_lightpath = NULL;

    for(auto &lp : exist_OFDM_light_path_list)
    {
        int require_slots = ceil(1.0 * event.bandwidth / lp->modulation_level / slot_capacity);
        OFDMTransceiver& transmitter = src_node.OFDMtransmitter[lp->transmitter_index[0]];
        int num_used_slots = transmitter.spectrum.slot_ed - transmitter.spectrum.slot_st + 1;

        if(source != lp->p_path.front() || destination != lp->p_path.back())
        {
            continue;
        }

        if(transceiver_slot_limit - num_used_slots < require_slots)
        {
            continue;
        }

        int slot_st;
        int slot_ed;

        // find the spectrum above the exisiting LightPath
        slot_st = lp->spectrum.slot_st - require_slots;
        slot_ed = lp->spectrum.slot_st - 1;
        if(slot_st >= 0 && slot_ed >= 0)
        {
            if(path_spectrum_available(lp->p_path, slot_st, slot_ed, p_graph) < 0) // spectrum available
            {
                current_weight = weigh_path_spectrum(lp->p_path, slot_st, slot_ed, p_graph);
                if(current_weight < best_path_spectrum.weight)
                {
                    best_path_spectrum.slot_st = slot_st;
                    // pre allocate the guardband slot for leaving adjant traffic
                    best_path_spectrum.slot_ed = slot_ed + 2;
                    best_path_spectrum.weight = current_weight;
                    best_existing_lightpath = lp;
                }
            }
        }

        // find the spectrum under the exisiting LightPath
        slot_st = lp->spectrum.slot_ed + 1;
        slot_ed = lp->spectrum.slot_ed + require_slots;
        if(slot_st < transceiver_slot_limit && slot_ed < transceiver_slot_limit)
        {
            if(path_spectrum_available(lp->p_path, slot_st, slot_ed, p_graph) < 0) // spectrum available
            {
                current_weight = weigh_path_spectrum(lp->p_path, slot_st, slot_ed, p_graph);
                if(current_weight < best_path_spectrum.weight)
                {
                    best_path_spectrum.slot_st = slot_st - 2;
                    best_path_spectrum.slot_ed = slot_ed;
                    best_path_spectrum.weight = current_weight;
                    best_existing_lightpath = lp;
                }
            }
        }
    } // end of for(auto &lp : exist_OFDM_light_path_list)

    if(best_path_spectrum.slot_st < 0){
        return NULL;
    }

    LightPath* path = new LightPath();

    path->type = LightPath::groomed_OFDM;
    path->modulation_level = best_existing_lightpath->modulation_level;
    path->p_path = best_existing_lightpath->p_path;
    path->weight = best_path_spectrum.weight;
    path->spectrum = best_path_spectrum;
    path->transmitter_index.front() = get_available_transceiver(src_node.OFDMtransmitter);
    path->receiver_index.back() = get_available_transceiver(dst_node.OFDMreceiver);

    // TODO Use this when build real light path
    // src_node.OFDMtransmitter[path->transmitter_index.front()].spectrum = best_path_spectrum;
    // dst_node.OFDMreceiver[path->receiver_index.back()].spectrum = best_path_spectrum;
    return path;
}

LightPath* get_best_optical_groomed_OFDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
{
    Phy_node& src_node = p_graph.get_node(source);
    Phy_node& dst_node = p_graph.get_node(destination);

    LightPath* best_existing_lightpath;
    list<CandidatePath>& c_path_list = p_graph.get_path_list(source, destination);
    Spectrum best_path_spectrum;
    double current_weight;

    best_path_spectrum.slot_st = -1;
    best_path_spectrum.slot_ed = -1;
    best_path_spectrum.weight = DBL_MAX;

    if(dst_node.num_available_OFDM_receiver < 1)
    {
        return NULL;
    }

    for(auto &c_path : c_path_list)
    {
        for(auto &lp : exist_OFDM_light_path_list)
        {
            int split_node_i = lp->p_path.back();
            int num_hops = lp->p_path.size();

            for(int node_i = 0; node_i < num_hops; node_i++)
            {
                if(c_path.path[node_i] != lp->p_path[node_i])
                {
                    split_node_i = node_i - 1;
                    break;
                }
            }

            if(split_node_i < 2)
            {
                continue;
            }

            if(split_node_i == (int)lp->p_path.size()-1) // use eletrical grooming instead
            {
                continue;
            }

            Path trunk(lp->p_path.begin(), lp->p_path.begin() + split_node_i);
            Path branch(lp->p_path.begin() + split_node_i, lp->p_path.end());

            int trunk_require_slots = ceil(1.0 * event.bandwidth / lp->modulation_level / slot_capacity);

            Spectrum transmitter_sp = src_node.OFDMtransmitter[lp->transmitter_index.front()].spectrum;

            int num_used_slots = transmitter_sp.slot_ed - transmitter_sp.slot_st + 1;

            if(transceiver_slot_limit - num_used_slots < trunk_require_slots)
            {
                continue;
            }

            int t_slot_st;  // start slot for trunk
            int t_slot_ed;  // end slot for trunk

            int b_slot_st;  // start slot for branch
            int b_slot_ed;  // end slot for branch

            // Trunk part
            t_slot_st = transmitter_sp.slot_st - trunk_require_slots;
            t_slot_ed = lp->spectrum.slot_st - 1;

            b_slot_st = t_slot_st;
            b_slot_ed = t_slot_ed + num_guardband_slot;

            if(t_slot_st >= 0)  // check for slot boundary
            {
                if(path_spectrum_available(trunk, t_slot_st, t_slot_ed, p_graph) < 0) // trunk spectrum available
                {
                    if(path_spectrum_available(branch, b_slot_st, b_slot_ed, p_graph) < 0) // branch spectrum available
                    {
                        current_weight = weigh_path_spectrum(lp->p_path, b_slot_st, b_slot_ed, p_graph);
                        if(current_weight < best_path_spectrum.weight)
                        {
                            best_path_spectrum.slot_st = b_slot_st;
                            best_path_spectrum.slot_ed = b_slot_ed;
                            best_path_spectrum.weight = current_weight;
                            best_existing_lightpath = lp;
                        }
                    }
                }
            }

            t_slot_st = lp->spectrum.slot_ed + 1;
            t_slot_ed = lp->spectrum.slot_ed + trunk_require_slots;

            b_slot_st = t_slot_st - num_guardband_slot;
            b_slot_ed = t_slot_ed;

            if(b_slot_ed < transceiver_slot_limit)  // check for slot boundary
            {
                if(path_spectrum_available(trunk, t_slot_st, t_slot_ed, p_graph) < 0) // trunk spectrum available
                {
                    if(path_spectrum_available(branch, b_slot_st, b_slot_ed, p_graph) < 0) // branch spectrum available
                    {
                        current_weight = weigh_path_spectrum(lp->p_path, b_slot_st, b_slot_ed, p_graph);
                        if(current_weight < best_path_spectrum.weight)
                        {
                            best_path_spectrum.slot_st = b_slot_st;
                            best_path_spectrum.slot_ed = b_slot_ed;
                            best_path_spectrum.weight = current_weight;
                            best_existing_lightpath = lp;
                        }
                    }
                }
            }
        }
    }

    if(best_path_spectrum.slot_st < 0)
    {
        return NULL;
    }

    LightPath* path = new LightPath();
    path->type = LightPath::groomed_OFDM;
    path->modulation_level = best_existing_lightpath->modulation_level;
    path->available_bitrate = best_existing_lightpath->available_bitrate;
    path->weight = best_path_spectrum.weight;
    path->spectrum = best_path_spectrum;
    path->transmitter_index = best_existing_lightpath->transmitter_index;
    path->receiver_index.back() = get_available_transceiver(dst_node.OFDMreceiver);
    return path;
}

Aux_node2Aux_link BellmanFordSP(Aux_graph& a_graph, Aux_node* s)
{
    Aux_node2Double distTo;
    Aux_node2Aux_link edgeTo;
    Aux_node2Bool onQueue;

    queue<Aux_node*> queue;

    distTo[s] = 0.0;

    Aux_node* v;

    // Bellman-Ford algorithm
    queue.push(s);
    onQueue[s] = true;
    while (!queue.empty())
    {
        v = queue.front();
        queue.pop();
        onQueue[v] = false;
        relax(v, distTo, edgeTo, onQueue, queue);
    }

    return edgeTo;
}

// if the distance to specific node is not element in map, assign the max value of double to it
double get_dist(Aux_node2Double& distTo, Aux_node* node)
{
    if(distTo.find(node) == distTo.end())
    {
        distTo[node] = DBL_MAX;
    }
    return distTo[node];
}

// relax vertex v and put other endpoints on queue if changed
void relax(Aux_node* v, Aux_node2Double& distTo, Aux_node2Aux_link& edgeTo, Aux_node2Bool& onQueue, queue<Aux_node*> queue)
{
    Aux_link* link;
    Aux_node* w;

    for(link = v->first_out; link != NULL; link = link->next_same_from)
    {
        w = link->to;
        if(get_dist(distTo, w) > get_dist(distTo, v) + link->weight)
        {
            distTo[w] = distTo[v] + link->weight;
            edgeTo[w] = link;
            if(!onQueue[w])
            {
                queue.push(w);
                onQueue[w] = true;
            }
        }
    }
}
