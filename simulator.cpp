#include "spectrum.hpp"
#include "light_path.hpp"
#include "graph.hpp"
#include "traffic.hpp"
#include "auxiliary.hpp"

#include <cfloat>
#include <ctime>
#include <queue>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <cmath>
#include <list>
#include <climits>

using namespace std;

clock_t start_clk;
clock_t start_clk_finding;
double clk_finding = 0;
clock_t start_clk_construction;
double clk_construction = 0;
clock_t start_clk_parsing;
double clk_parsing = 0;

float unicast_percentage = 1.0;
int num_requests = 1000;
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
int blocked_bandwidth = 0;
int total_bandwidth = 0;
int trail_usage_count = 0;
int trail_usage_count_back = 0;

double total_bandwidth_utilization=0;
double used_bandwidth_utilization=0;
double total_bandwidth_utilization_back=0;
double used_bandwidth_utilization_back=0;

// edge weight
double transceiver_weight = 0.3;
double used_transceiver_weight = 0.2;
double OFDM_transceiver_weight = 0.3;
double used_OFDM_transceiver_weight = 0.2;
double OEO_weight = 0.3;

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

vector< vector<LightPath*> > request2lightpath;


typedef unordered_map<Aux_node*, double> Aux_node2Double;
typedef unordered_map<Aux_node*, Aux_link*> Aux_node2Aux_link;
typedef unordered_map<Aux_node*, bool> Aux_node2Bool;

void path_parsing(Phy_graph& p_graph, Aux_node2Aux_link& result, Aux_node* aux_source, Aux_node* aux_destination, Event& event);
void reset_auxiliary_graph();

Aux_node2Aux_link BellmanFordSP(Aux_node* s);
double get_dist(Aux_node2Double& distTo, Aux_node* node);
void relax(Aux_node* v, Aux_node2Double& distTo, Aux_node2Aux_link& edgeTo, Aux_node2Bool& onQueue, queue<Aux_node*>& queue);

void construct_candidate_path(Event& event, Phy_graph& p_graph, Aux_graph& a_graph);
void construct_exist_path(Event& event, Aux_graph& a_graph);

void build_candidate_link(Aux_graph& a_graph, LightPath* lpath);
int get_available_OFDM_transceiver(vector<OFDMTransceiver> transceivers);
LightPath* get_best_new_OTDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph);
LightPath* get_best_new_OFDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph);
LightPath* get_best_optical_groomed_OFDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph);
LightPath* get_best_electrical_groomed_OFDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph);
void print_result();

char* graph_file = (char*) "NSFnet.txt";
char* source_file = (char*) "NSFnet_source.txt";
char* traffic_file = (char*) "NSFnet_traffic.txt";

int main(int argc, char *argv[])
{
    if( argc > 5 )
    {
        cout << "\nToo many arguments,EX: simulation labmda rquest_number" << "\n";
        return 1;
    }
    if( argc > 1 )
    {
        if( atoi( argv[ 1 ] ) > 0 )
        {
            traffic_lambda = atoi( argv[ 1 ] );
        }
        else
        {
            cout << "\nError parameter: 1" << "\n";
            return 1;
        }
    }
    if( argc > 2 )
    {
        if( atoi( argv[ 2 ] ) > 0 )
        {
            num_requests = atoi( argv[ 2 ] );
        }
        else
        {
            cout << "\nError parameter: 2" << "\n";
            return 1;
        }
    }
    if( argc > 3 )
    {
        if( atoi( argv[ 3 ] ) > 0 )
        {
            num_OFDM_transceiver= atoi( argv[ 3 ] );
            num_OTDM_transceiver= atoi( argv[ 3 ] );
        }
        else
        {
            cout << "\nError parameter: 3" << "\n";
            return 1;
        }
    }
    if( argc > 4 )
    {
        switch( atoi( argv[4] ) )
        {
        case 1:
            graph_file = (char*) "USnet.txt";
            source_file = (char*) "USnet_source.txt";
            traffic_file = (char*) "USnet_traffic.txt";
            break;
        default:
            break;
        }
    }

    start_clk = clock();

    Graph_info g_info;
    g_info.graph_file = graph_file;
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
    t_info.source_file = source_file;
    t_info.traffic_file = traffic_file;
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

    request2lightpath.resize(num_requests);

    // Phy_node& a = p_graph.get_node(0);
    // Phy_link& b = p_graph.get_link(0,1);

    // cout << a.degree << '\n';
    // cout << b.source << '\n';

    Auxiliary_info a_info;
    a_info.num_nodes = num_nodes;
    a_info.num_slots = num_slots;
    a_info.transceiver_weight = transceiver_weight;
    a_info.used_transceiver_weight = used_transceiver_weight;
    a_info.OFDM_transceiver_weight = OFDM_transceiver_weight;
    a_info.used_OFDM_transceiver_weight = used_OFDM_transceiver_weight;
    a_info.OEO_weight = OEO_weight;
    Aux_graph a_graph(a_info);

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

    // cout << p_graph.get_node(0).OFDMtransmitter[0].sub_transceiver.size() << '\n';
    // cout << p_graph.get_node(0).OFDMtransmitter.size() << '\n';

    while( !traffic.empty() )
    {
        Event event = traffic.next_event();
        if(event.type == Event::arrival)
        {
            cout << "arrival :\nid = " << event.request_id << "\nsource = " << event.source << "\ndest = " << *event.destination.begin() << "\nbandwidth = " << event.bandwidth << "\narrivaltime = " << event.arrival_time << "\n\n";
            start_clk_construction = clock();
            construct_candidate_path(event, p_graph, a_graph);
            construct_exist_path(event, a_graph);
            clk_construction += (double) ( clock() - start_clk_construction ) / CLOCKS_PER_SEC;
            start_clk_finding = clock();
            Aux_node* aux_source = a_graph.get_adding_node(event.source);
            Aux_node* aux_destination = a_graph.get_dropping_node(*(event.destination.begin()));
            Aux_node2Aux_link result = BellmanFordSP(aux_source);
            clk_finding += (double) ( clock() - start_clk_finding ) / CLOCKS_PER_SEC;
            start_clk_parsing = clock();
            if(result[aux_destination] != NULL) // Accepted
            {
                path_parsing(p_graph, result, aux_source, aux_destination, event);
                accepted_requests++;
            }
            else
            {
                blocked_requests++;
                blocked_bandwidth += event.bandwidth;
            }
            clk_parsing += (double) ( clock() - start_clk_parsing ) / CLOCKS_PER_SEC;
            total_bandwidth += event.bandwidth;
            start_clk_construction = clock();
            reset_auxiliary_graph();
            clk_construction += (double) ( clock() - start_clk_construction ) / CLOCKS_PER_SEC;
        }
        else // if(event.type == Event::departure)
        {
            cout << "departure :\nid = " << event.request_id << "\nsource = " << event.source << "\ndest = " << *event.destination.begin() << "\nbandwidth = " << event.bandwidth << "\narrivaltime = " << event.arrival_time << "\n\n";
            for(auto &lp : request2lightpath[event.request_id])
            {
                lp->requests.erase(event.request_id);
                if(lp->requests.empty())
                {
                    int slot_st;
                    int slot_ed;
                    Path& path = lp->p_path;
                    switch(lp->type)
                    {
                    case LightPath::new_OTDM:
                        slot_st = lp->spectrum.slot_st;
                        slot_ed = lp->spectrum.slot_ed;

                        for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
                        {
                            int from = path[node_i];
                            int to = path[node_i + 1];
                            Phy_link& link = p_graph.get_link(from, to);

                            for(int i = slot_st; i < slot_ed; i++)
                            {
                                link.slot[i] = -1;
                            }
                        }

                        for(unsigned int i = 0; i < lp->transmitter_index.size(); i++)
                        {
                            if(lp->transmitter_index[i] != -1)
                            {
                                p_graph.get_node(path[i]).num_available_transmitter++;
                            }
                        }

                        for(unsigned int i = 0; i < lp->receiver_index.size(); i++)
                        {
                            if(lp->receiver_index[i] != -1)
                            {
                                p_graph.get_node(path[i]).num_available_receiver++;
                            }
                        }
                        delete lp;
                        exist_OTDM_light_path_list.remove(lp);
                        break;
                    case LightPath::electrical:
                    case LightPath::groomed_OFDM:
                    case LightPath::new_OFDM:
                        slot_st = lp->spectrum.slot_st;
                        slot_ed = lp->spectrum.slot_ed;

                        Phy_node& src_node = p_graph.get_node(path.front());
                        Phy_node& dst_node = p_graph.get_node(path.back());
                        src_node.OFDMtransmitter[lp->transmitter_index.front()].num_available_sub_transceiver++;
                        dst_node.OFDMreceiver[lp->receiver_index.back()].num_available_sub_transceiver++;

                        for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
                        {
                            int from = path[node_i];
                            int to = path[node_i + 1];
                            Phy_link& link = p_graph.get_link(from, to);

                            for(int i = slot_st; i < slot_ed; i++)
                            {
                                link.slot[i] = -1;
                            }
                        }

                        if(src_node.OFDMtransmitter[lp->transmitter_index.front()].num_available_sub_transceiver >= transceiver_connection_limit)
                        {
                            src_node.OFDMtransmitter[lp->transmitter_index.front()].spectrum.slot_st = -1;
                            src_node.OFDMtransmitter[lp->transmitter_index.front()].spectrum.slot_ed = -1;
                            src_node.OFDMtransmitter[lp->transmitter_index.front()].in_used = false;
                            src_node.OFDMtransmitter[lp->transmitter_index.front()].num_available_sub_transceiver = transceiver_connection_limit;
                            src_node.num_available_OFDM_transmitter++;
                        }

                        if(dst_node.OFDMreceiver[lp->receiver_index.back()].num_available_sub_transceiver >= transceiver_connection_limit)
                        {
                            dst_node.OFDMreceiver[lp->receiver_index.back()].spectrum.slot_st = -1;
                            dst_node.OFDMreceiver[lp->receiver_index.back()].spectrum.slot_ed = -1;
                            dst_node.OFDMreceiver[lp->receiver_index.back()].in_used = false;
                            dst_node.OFDMreceiver[lp->receiver_index.back()].num_available_sub_transceiver = transceiver_connection_limit;
                            dst_node.num_available_OFDM_receiver++;
                        }
                        delete lp;
                        exist_OFDM_light_path_list.remove(lp);
                        break;
                    }
                }
                else
                {
                    lp->available_bitrate -= event.bandwidth;
                }
            }
        }
    }
    print_result();
    return 0;
}

void construct_exist_path(Event& event, Aux_graph& a_graph)
{
    for(auto &lp : exist_OTDM_light_path_list)
    {
        if(lp->available_bitrate < event.bandwidth)
        {
            continue;
        }

        Aux_node* a_node;
        Aux_node* d_node;
        Aux_node* t_node;
        Aux_node* r_node;
        Aux_link* aux_link;

        lp->transmitting_node_list.reserve(lp->p_path.size());
        lp->receiving_node_list.reserve(lp->p_path.size());

        for(unsigned int i = 0; i < lp->p_path.size(); i++)
        {
            int phy_node_id = lp->p_path[i];

            a_node = a_graph.get_adding_node(phy_node_id);
            d_node = a_graph.get_dropping_node(phy_node_id);

            t_node = a_graph.create_aux_node(phy_node_id, Aux_node::transmitting_node);
            lp->transmitting_node_list.push_back(t_node);

            r_node = a_graph.create_aux_node(phy_node_id, Aux_node::receiving_node);
            lp->receiving_node_list.push_back(r_node);

            aux_link = a_graph.create_aux_link(r_node, t_node, 0.0 - lp->weight, Aux_link::pass_through_link);
            lp->aux_link_list.push_back(aux_link);
            aux_link->light_path = lp;              // make aux_link track light path

            if(lp->transmitter_index[i] != -1)
            {
                aux_link = a_graph.create_aux_link(a_node, t_node, used_transceiver_weight, Aux_link::adding_link);
            }
            else
            {
                aux_link = a_graph.create_aux_link(a_node, t_node, transceiver_weight, Aux_link::virtual_adding_link);
            }
            lp->aux_link_list.push_back(aux_link);
            aux_link->light_path = lp;              // make aux_link track light path


            if(lp->receiver_index[i] != -1)
            {
                aux_link = a_graph.create_aux_link(r_node, d_node, used_transceiver_weight, Aux_link::dropping_link);
            }
            else
            {
                aux_link = a_graph.create_aux_link(r_node, d_node, transceiver_weight, Aux_link::virtual_dropping_link);
            }
            lp->aux_link_list.push_back(aux_link);
            aux_link->light_path = lp;              // make aux_link track light path
        }

        for(unsigned int i = 0; i < lp->p_path.size() - 1; i++)
        {
            t_node = lp->transmitting_node_list[i];
            r_node = lp->receiving_node_list[i + 1];
            aux_link = a_graph.create_aux_link(t_node, r_node, lp->weight, Aux_link::spectrum_link);
            // cout << "lp : " << lp << "  link: " << aux_link << "\n";
            lp->aux_link_list.push_back(aux_link);
            aux_link->light_path = lp;              // make aux_link track light path
        }
    }
}

void reset_auxiliary_graph()
{
    for(auto &lp : exist_OTDM_light_path_list)
    {
        for(auto &aux_node : lp->transmitting_node_list)
        {
            delete aux_node;
        }
        lp->transmitting_node_list.clear();
        for(auto &aux_node : lp->receiving_node_list)
        {
            delete aux_node;
        }
        lp->receiving_node_list.clear();
        lp->aux_link_list.clear();
    }
    for(auto &lp : exist_OFDM_light_path_list)
    {
        for(auto &aux_link : lp->aux_link_list)
        {
            delete aux_link;
        }
        lp->aux_link_list.clear();
    }
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

            LightPath* new_OTDM_lp = get_best_new_OTDM_light_path(source, destination, event, p_graph);
            LightPath* new_OFDM_lp = get_best_new_OFDM_light_path(source, destination, event, p_graph);
            LightPath* groomed_OFDM_lp = get_best_optical_groomed_OFDM_light_path(source, destination, event, p_graph);
            LightPath* electrical_lp = get_best_electrical_groomed_OFDM_light_path(source, destination, event, p_graph);
            build_candidate_link(a_graph, new_OTDM_lp);
            build_candidate_link(a_graph, new_OFDM_lp);
            build_candidate_link(a_graph, groomed_OFDM_lp);
            build_candidate_link(a_graph, electrical_lp);
        }
    }
}

void build_candidate_link(Aux_graph& a_graph, LightPath* lpath)
{
    if(lpath == NULL)
    {
        return;
    }

    int source = lpath->p_path.front();
    int destination = lpath->p_path.back();

    Aux_node* v_t_node;
    Aux_node* v_r_node;

    switch (lpath->type)
    {
    case LightPath::new_OTDM:
        v_t_node = a_graph.get_new_OTDM_virtual_transmitting_node(source);
        v_r_node = a_graph.get_new_OTDM_virtual_receiving_node(destination);
        break;
    case LightPath::new_OFDM:
        v_t_node = a_graph.get_new_OFDM_virtual_transmitting_node(source);
        v_r_node = a_graph.get_new_OFDM_virtual_receiving_node(destination);
        break;
    case LightPath::groomed_OFDM:

        v_t_node = a_graph.get_groomed_OFDM_virtual_transmitting_node(source);
        v_r_node = a_graph.get_groomed_OFDM_virtual_receiving_node(destination);
        break;
    case LightPath::electrical:
        v_t_node = a_graph.get_electrical_virtual_transmitting_node(source);
        v_r_node = a_graph.get_electrical_virtual_receiving_node(destination);
        break;
    default:
        cerr << "undefined LightPath type\n";
        break;
    }

    Aux_link* c_link = a_graph.create_aux_link(v_t_node, v_r_node, lpath->weight, Aux_link::candidate_link);

    c_link->light_path = lpath;              // make aux_link track light path

    lpath->aux_link_list.push_back(c_link);  // make light path track aux_link

    candidate_light_path_list.push_back(lpath);
}

void build_light_path(Phy_graph p_graph, LightPath* candidate_path, Aux_node* aux_source, Aux_node* aux_destination, int request_id)
{
    Phy_node& src_node = p_graph.get_node(aux_source->phy_id);
    Phy_node& dst_node = p_graph.get_node(aux_destination->phy_id);
    LightPath* new_path = new LightPath();

    request2lightpath[request_id].push_back(new_path);
    new_path->requests.insert(request_id);
    new_path->modulation_level = candidate_path->modulation_level;
    new_path->available_bitrate = candidate_path->available_bitrate;
    new_path->weight = candidate_path->weight;
    new_path->spectrum = candidate_path->spectrum;
    new_path->p_path = candidate_path->p_path;


    int slot_st = new_path->spectrum.slot_st;
    int slot_ed = new_path->spectrum.slot_ed;
    Path& path = new_path->p_path;

    switch(candidate_path->type)
    {
    case LightPath::new_OTDM:

        new_path->transmitter_index.resize(path.size(), -1);
        new_path->receiver_index.resize(path.size(), -1);
        new_path->transmitter_index.front() = 1;
        new_path->receiver_index.back() = 1;
        src_node.num_available_transmitter--;
        dst_node.num_available_receiver--;

        for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
        {
            int from = path[node_i];
            int to = path[node_i + 1];
            Phy_link& link = p_graph.get_link(from, to);

            link.slot[slot_st] = 0;
            link.slot[slot_ed] = 0;

            for(int i = slot_st + 1; i < slot_ed - 1; i++)
            {
                link.slot[i] = 1;
            }
        }
        exist_OTDM_light_path_list.push_back(new_path);
        break;

    case LightPath:: new_OFDM:
        new_path->transmitter_index.resize(path.size(), -1);
        new_path->receiver_index.resize(path.size(), -1);
        new_path->transmitter_index.front() = get_available_OFDM_transceiver(src_node.OFDMtransmitter);
        src_node.num_available_OFDM_transmitter--;
        new_path->receiver_index.back() = get_available_OFDM_transceiver(dst_node.OFDMreceiver);
        dst_node.num_available_OFDM_receiver--;
        src_node.OFDMtransmitter[new_path->transmitter_index.front()].spectrum = candidate_path->spectrum;
        src_node.OFDMtransmitter[new_path->transmitter_index.front()].num_available_sub_transceiver--;
        src_node.OFDMtransmitter[new_path->transmitter_index.front()].in_used = true;
        dst_node.OFDMreceiver[new_path->receiver_index.back()].spectrum = candidate_path->spectrum;
        dst_node.OFDMreceiver[new_path->receiver_index.back()].num_available_sub_transceiver--;
        dst_node.OFDMreceiver[new_path->receiver_index.back()].in_used = true;

        for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
        {
            int from = path[node_i];
            int to = path[node_i + 1];
            Phy_link& link = p_graph.get_link(from, to);

            link.slot[slot_st] = 0;
            link.slot[slot_ed] = 0;
            for(int i = slot_st + 1; i < slot_ed - 1; i++)
            {
                link.slot[i] = 1;
            }
        }
        exist_OFDM_light_path_list.push_back(new_path);
        break;

    case LightPath:: groomed_OFDM:
        new_path->transmitter_index = candidate_path->transmitter_index;
        new_path->receiver_index.resize(path.size(), -1);
        new_path->receiver_index.back() = get_available_OFDM_transceiver(dst_node.OFDMreceiver);
        dst_node.OFDMreceiver[new_path->receiver_index.back()].spectrum = candidate_path->spectrum;
        dst_node.OFDMreceiver[new_path->receiver_index.back()].num_available_sub_transceiver--;
        dst_node.OFDMreceiver[new_path->receiver_index.back()].in_used = true;
        dst_node.num_available_OFDM_receiver--;
        src_node.OFDMtransmitter[new_path->transmitter_index.front()].num_available_sub_transceiver--;
        if(src_node.OFDMtransmitter[new_path->transmitter_index.front()].spectrum.slot_st > slot_st)
        {
            src_node.OFDMtransmitter[new_path->transmitter_index.front()].spectrum.slot_st = slot_st;
        }
        if(src_node.OFDMtransmitter[new_path->transmitter_index.front()].spectrum.slot_ed < slot_ed)
        {
            src_node.OFDMtransmitter[new_path->transmitter_index.front()].spectrum.slot_ed = slot_ed;
        }
        for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
        {
            int from = path[node_i];
            int to = path[node_i + 1];
            Phy_link& link = p_graph.get_link(from, to);

            link.slot[slot_st] = 0;
            link.slot[slot_ed] = 0;
            for(int i = slot_st + 1; i < slot_ed - 1; i++)
            {
                link.slot[i] = 1;
            }
        }
        exist_OFDM_light_path_list.push_back(new_path);
        break;

    case LightPath:: electrical:
        new_path->transmitter_index = candidate_path->transmitter_index;
        new_path->receiver_index = candidate_path->receiver_index;
        src_node.OFDMtransmitter[new_path->transmitter_index.front()].num_available_sub_transceiver--;
        dst_node.OFDMreceiver[new_path->receiver_index.back()].num_available_sub_transceiver--;
        if(src_node.OFDMtransmitter[new_path->transmitter_index.front()].spectrum.slot_st > slot_st)
        {
            src_node.OFDMtransmitter[new_path->transmitter_index.front()].spectrum.slot_st = slot_st;
        }
        if(src_node.OFDMtransmitter[new_path->transmitter_index.front()].spectrum.slot_ed < slot_ed)
        {
            src_node.OFDMtransmitter[new_path->transmitter_index.front()].spectrum.slot_ed = slot_ed;
        }
        if(dst_node.OFDMreceiver[new_path->receiver_index.back()].spectrum.slot_st > slot_st)
        {
            dst_node.OFDMreceiver[new_path->receiver_index.back()].spectrum.slot_st = slot_st;
        }
        if(dst_node.OFDMreceiver[new_path->receiver_index.back()].spectrum.slot_ed < slot_ed)
        {
            dst_node.OFDMreceiver[new_path->receiver_index.back()].spectrum.slot_ed = slot_ed;
        }

        for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
        {
            int from = path[node_i];
            int to = path[node_i + 1];
            Phy_link& link = p_graph.get_link(from, to);

            link.slot[slot_st] = 0;
            link.slot[slot_ed] = 0;
            for(int i = slot_st + 1; i < slot_ed - 1; i++)
            {
                link.slot[i] = 1;
            }
        }
        exist_OFDM_light_path_list.push_back(new_path);
        break;

    default:
        cerr << "light path type error\n";
        break;
    }
}

void path_parsing(Phy_graph& p_graph, Aux_node2Aux_link& result, Aux_node* aux_source, Aux_node* aux_destination, Event& event)
{
    Aux_link* aux_link = result[aux_destination];
    Aux_node* aux_node = aux_link->from;
    while(aux_node != aux_source)
    {
        switch(aux_link->type)
        {
        case Aux_link::candidate_link:
            build_light_path(p_graph, aux_link->light_path, aux_source, aux_destination, event.request_id);
            break;
        case Aux_link::spectrum_link:
            // cout << aux_link << "\n";
            // cout << aux_link->light_path << "\n";
            // cout << aux_link->light_path->requests.size() << "\n";
            if(aux_link->light_path->requests.find(event.request_id) == aux_link->light_path->requests.end())
            {
                request2lightpath[event.request_id].push_back(aux_link->light_path);
                aux_link->light_path->requests.insert(event.request_id);
                aux_link->light_path->available_bitrate -= event.bandwidth;
            }
            break;
        case Aux_link::grooming_link:
            break;
        case Aux_link::adding_link:
            break;
        case Aux_link::dropping_link:
            break;
        case Aux_link::pass_through_link:
            break;
        case Aux_link::virtual_adding_link:
            if(aux_link->light_path == NULL)
            {
                break;
            }
            if(aux_link->light_path->type == LightPath::new_OTDM)
            {
                unsigned int i;
                for(i = 0; i < aux_link->light_path->p_path.size(); i++)
                {
                    if(aux_node->phy_id == aux_link->light_path->p_path[i])
                    {
                        break;
                    }
                }
                aux_link->light_path->transmitter_index[i] = 1;
                p_graph.get_node(aux_node->phy_id).num_available_transmitter--;
            }
            break;
        case Aux_link::virtual_dropping_link:
            if(aux_link->light_path == NULL)
            {
                break;
            }
            if(aux_link->light_path->type == LightPath::new_OTDM)
            {
                unsigned int i;
                for(i = 0; i < aux_link->light_path->p_path.size(); i++)
                {
                    if(aux_node->phy_id == aux_link->light_path->p_path[i])
                    {
                        break;
                    }
                }
                aux_link->light_path->receiver_index[i] = 1;
                p_graph.get_node(aux_node->phy_id).num_available_receiver--;
            }
            break;
        default:
            cerr << "aux_link type error\n";
            break;
        }
        aux_link = result[aux_node];
        aux_node = aux_link->from;
    }
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
    return transceiver_slot_limit*2-reserved_coefficent * distance + cut_coeffcient * num_cut + align_coeffcient * num_align;
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
        slot_st = (next_start < 0)? slot_st+1:next_start;
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
    Path best_p_path;

    best_path_spectrum.slot_st = -1;
    best_path_spectrum.slot_ed = -1;
    best_path_spectrum.weight = DBL_MAX;

    for(auto &c_path : c_path_list)
    {
        int require_slots = num_guardband_slot * 2 + ceil(1.0 * event.bandwidth / c_path.modulation_level / slot_capacity);
        path_specturm = find_best_spectrum(c_path.path, require_slots, p_graph);
        if(best_path_spectrum.weight > path_specturm.weight){
            best_p_path = c_path.path;
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
    path->p_path = best_p_path;
    path->modulation_level = modulation_level;
    path->available_bitrate = available_bitrate;
    path->weight = best_path_spectrum.weight;
    path->spectrum = best_path_spectrum;
    return path;
}

int get_available_OFDM_transceiver(vector<OFDMTransceiver> transceivers)
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
    Path best_p_path;

    best_path_spectrum.slot_st = -1;
    best_path_spectrum.slot_ed = -1;
    best_path_spectrum.weight = DBL_MAX;

    for(auto &c_path : c_path_list)
    {
        int require_slots = num_guardband_slot * 2 + ceil(1.0 * event.bandwidth / c_path.modulation_level / slot_capacity);
        path_specturm = find_best_spectrum(c_path.path, require_slots, p_graph);
        if(best_path_spectrum.weight > path_specturm.weight){
            best_p_path = c_path.path;
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
    path->p_path = best_p_path;
    path->modulation_level = modulation_level;
    path->available_bitrate = available_bitrate;
    path->weight = best_path_spectrum.weight;
    path->spectrum = best_path_spectrum;
    return path;
}

LightPath* get_best_electrical_groomed_OFDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
{
    Phy_node& src_node = p_graph.get_node(source);
    // Phy_node& dst_node = p_graph.get_node(destination);

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

        if(transmitter.num_available_sub_transceiver < 1)
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
                    // pre allocate the guardband slot for leaving adjacent traffic
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
    path->transmitter_index = best_existing_lightpath->transmitter_index;
    path->receiver_index = best_existing_lightpath->receiver_index;
    path->weight = best_path_spectrum.weight;
    path->spectrum = best_path_spectrum;

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
    Path best_p_path;

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
            int split_node_i;
            unsigned int num_hops = lp->p_path.size();

            OFDMTransceiver& transmitter = src_node.OFDMtransmitter[lp->transmitter_index[0]];

            if(transmitter.num_available_sub_transceiver < 1)
            {
                continue;
            }

            if(c_path.path.size() < lp->p_path.size())
            {
                split_node_i = c_path.path.back();
                num_hops = c_path.path.size();
            }
            else
            {
                split_node_i = lp->p_path.back();
                num_hops = lp->p_path.size();
            }

            for(unsigned int node_i = 0; node_i < num_hops; node_i++)
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
                            best_p_path = c_path.path;
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
                            best_p_path = c_path.path;
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
    path->p_path = best_p_path;
    path->modulation_level = best_existing_lightpath->modulation_level;
    path->available_bitrate = best_existing_lightpath->available_bitrate;
    path->weight = best_path_spectrum.weight;
    path->spectrum = best_path_spectrum;
    path->transmitter_index = best_existing_lightpath->transmitter_index;
    return path;
}

Aux_node2Aux_link BellmanFordSP(Aux_node* s)
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
        // cout << "relax node: "<< v->phy_id << "  <<< " << v->type << "\n";
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
void relax(Aux_node* v, Aux_node2Double& distTo, Aux_node2Aux_link& edgeTo, Aux_node2Bool& onQueue, queue<Aux_node*>& queue)
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
                // cout << "other node: "<< w->phy_id << "  <<< " << w->type << "\n";
                queue.push(w);
                onQueue[w] = true;
            }
        }
    }
}

void print_result()
{
    cout << "Output result\n";
    ostringstream filename;

    filename << "result_" << traffic_lambda << "_" << num_requests << "_" << graph_file;

    ofstream ofs (filename.str(), ofstream::out);

    ofs << "Bandwidth Blocking Ratio: "<<(double)blocked_bandwidth/total_bandwidth<<endl;
    ofs << "Blocked Requests: "<<blocked_requests<<endl;
    ofs << "Blocking Probability: "<<(double)blocked_requests/num_requests << endl;
    ofs << "Load:"<< traffic_lambda << endl << endl;

    ofs << "Elapsed time: " << (double) ( clock() - start_clk ) / CLOCKS_PER_SEC << " seconds" << endl;
    ofs << "Time spending for graph construction: "<<clk_construction<<" seconds" << endl;
    ofs << "Time spending for path finding: "<<clk_finding<<" seconds" << endl;
    ofs << "Time spending for path parsing: "<<clk_parsing<<" seconds" << endl<<endl;

    ofs.close();
}
