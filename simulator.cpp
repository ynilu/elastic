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

int num_batch = 10;
int hop_limit = 7;
float unicast_percentage = 1.0;
int num_requests = 1000;
int num_slots = 320;
int num_nodes;
int traffic_lambda = 1000;
int traffic_mu = 1;
int num_transceiver = 800;
int num_OTDM_transceiver = 400;
int num_OFDM_transceiver = 400;
int slot_capacity = 250;
int transceiver_slot_limit = 10;
int transceiver_connection_limit = 10;
int num_guardband_slot = 1;
int enable_OTDM = 1;
int OTDM_threshold = 18;
int transmission_distance_16QAM = 1800;
int transmission_distance_8QAM = 3600;
int transmission_distance_QPSK = 7200;

// request bandwidth share
int OC1_share = 0;
int OC3_share = 0;
int OC9_share = 0;
int OC12_share = 0;
int OC18_share = 2;
int OC24_share = 2;
int OC36_share = 2;
int OC48_share = 2;
int OC192_share = 1;
int OC768_share = 1;
int OC3072_share = 1;

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
int num_OEO = 0;
int num_OFDM_lightpath_use = 0;
int num_OTDM_lightpath_use = 0;
int total_bandwidth = 0;
int total_network_bandwidth=0;
vector<int> accepted_requests_BM;
vector<int> blocked_requests_BM;
vector<int> blocked_bandwidth_BM;
vector<int> num_OEO_BM;
vector<int> num_OFDM_lightpath_use_BM;
vector<int> num_OTDM_lightpath_use_BM;
double total_bandwidth_utilization;
double used_bandwidth_utilization;
double wasted_bandwidth_utilization;
double guardband_bandwidth_utilization;
double end_time;

// edge weight
double eps = 0.03;
double transceiver_weight = (1-eps) * 0.1;
double used_transceiver_weight = (1-eps) * 0.01;
double OFDM_transceiver_weight = (1-eps) * 0.1;
double used_OFDM_transceiver_weight = (1-eps) * 0.01;
double OEO_weight = (1-eps) * 0.1;

// police coeffcient
double reserved_coefficent = 1;
double cut_coeffcient = 1;
double align_coeffcient = 1;

list<LightPath*> candidate_light_path_list;
list<LightPath*> exist_OTDM_light_path_list;
list<LightPath*> exist_OFDM_light_path_list;

vector< vector<LightPath*> > request2lightpath;

char* graph_file = (char*) "NSFnet.txt";
char* source_file = (char*) "NSFnet_source.txt";
char* traffic_file = (char*) "NSFnet_traffic.txt";


typedef unordered_map<Aux_node*, double> Aux_node2Double;
typedef unordered_map<Aux_node*, Aux_link*> Aux_node2Aux_link;
typedef unordered_map<Aux_node*, bool> Aux_node2Bool;

void construct_exist_path(Event& event, Phy_graph& p_graph, Aux_graph& a_graph);
void construct_candidate_path(Event& event, Phy_graph& p_graph, Aux_graph& a_graph);
void reset_auxiliary_graph();
void build_candidate_link(Aux_graph& a_graph, LightPath* lpath);
void build_light_path(Phy_graph& p_graph, LightPath* candidate_path, Aux_node* aux_source, Aux_node* aux_destination, Event& event);
void path_parsing(Phy_graph& p_graph, Aux_node2Aux_link& result, Aux_node* aux_source, Aux_node* aux_destination, Event& event);

int num_spectrum_available(Phy_link& link, int slot_st, int slot_ed);
int spectrum_available(Phy_link& link, int slot_st, int slot_ed);
int path_spectrum_available(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph);
int get_distance(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph);
int get_cut_num(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph);
int get_align_num(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph);
double weigh_path_spectrum(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph);
Spectrum find_best_spectrum(Path& path, int require_slots, Phy_graph& p_graph);

int get_available_OFDM_transceiver(vector<OFDMTransceiver>& transceivers);
LightPath* get_best_OTDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph);
LightPath* get_best_OFDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph);
LightPath* get_best_OFDM_WB_light_path(int source, int destination, Event& event, Phy_graph& p_graph);
LightPath* get_best_OFDM_WOB_light_path(int source, int destination, Event& event, Phy_graph& p_graph);

Aux_node2Aux_link BellmanFordSP(Aux_node* s);
double get_dist(Aux_node2Double& distTo, Aux_node* node);
void relax(Aux_node* v, Aux_node2Double& distTo, Aux_node2Aux_link& edgeTo, Aux_node2Bool& onQueue, queue<Aux_node*>& queue);
void print_result(Traffic traffic);


int main(int argc, char *argv[])
{
    if( argc > 6 )
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
        if( atoi( argv[ 3 ] ) == 0 )
        {
            enable_OTDM = 0;
        }
    }
    if( argc > 4 )
    {
        if( atoi( argv[ 4 ] ) > 0 )
        {
            num_transceiver = atoi( argv[ 4 ] );
        }
        else
        {
            cout << "\nError parameter: 4" << "\n";
            return 1;
        }
    }
    if( argc > 5 )
    {
        switch( atoi( argv[5] ) )
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

    if(num_transceiver < 800)
    {
        OEO_weight = hop_limit;
    }

    if(enable_OTDM)
    {
        num_OFDM_transceiver = num_transceiver / 2;
        num_OTDM_transceiver = num_transceiver / 2;
    }
    else
    {
        num_OFDM_transceiver = num_transceiver;
        num_OTDM_transceiver = 0;
    }

    start_clk = clock();

    accepted_requests_BM.resize(num_batch, 0);
    blocked_requests_BM.resize(num_batch, 0);
    blocked_bandwidth_BM.resize(num_batch, 0);
    num_OEO_BM.resize(num_batch, 0);
    num_OFDM_lightpath_use_BM.resize(num_batch, 0);
    num_OTDM_lightpath_use_BM.resize(num_batch, 0);

    Graph_info g_info;
    g_info.graph_file = graph_file;
    g_info.num_slots = num_slots;
    g_info.num_OTDM_transceiver = num_OTDM_transceiver;
    g_info.num_OFDM_transceiver = num_OFDM_transceiver;
    g_info.slot_capacity = slot_capacity;
    g_info.transceiver_slot_limit = transceiver_slot_limit;
    g_info.transceiver_connection_limit = transceiver_connection_limit;
    g_info.transmission_distance_16QAM = transmission_distance_16QAM;
    g_info.transmission_distance_8QAM = transmission_distance_8QAM;
    g_info.transmission_distance_QPSK = transmission_distance_QPSK;

    Phy_graph p_graph(g_info);

    cout << "num_nodes : " << p_graph.node_list.size() << "\n";
    num_nodes = p_graph.node_list.size();

    Traffic_info t_info;
    t_info.source_file = source_file;
    t_info.traffic_file = traffic_file;
    t_info.num_nodes = p_graph.node_list.size();
    t_info.num_requests = num_requests;
    t_info.OC1_share = OC1_share;
    t_info.OC3_share = OC3_share;
    t_info.OC9_share = OC9_share;
    t_info.OC12_share = OC12_share;
    t_info.OC18_share = OC18_share;
    t_info.OC24_share = OC24_share;
    t_info.OC36_share = OC36_share;
    t_info.OC48_share = OC48_share;
    t_info.OC192_share = OC192_share;
    t_info.OC768_share = OC768_share;
    t_info.OC3072_share = OC3072_share;
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

    Auxiliary_info a_info;
    a_info.num_nodes = num_nodes;
    a_info.num_slots = num_slots;
    a_info.transceiver_weight = transceiver_weight;
    a_info.used_transceiver_weight = used_transceiver_weight;
    a_info.OFDM_transceiver_weight = OFDM_transceiver_weight;
    a_info.used_OFDM_transceiver_weight = used_OFDM_transceiver_weight;
    a_info.OEO_weight = OEO_weight;
    Aux_graph a_graph(a_info);


    while( !traffic.empty() )
    {
        Event event = traffic.next_event();
        if(event.type == Event::arrival)
        {
            cout << "arrival :\nid = " << event.request_id << "\nsource = " << event.source << "\ndest = " << *event.destination.begin() << "\nbandwidth = " << event.bandwidth << "\narrivaltime = " << event.arrival_time << "\n\n";
            start_clk_construction = clock();
            construct_candidate_path(event, p_graph, a_graph);
            construct_exist_path(event, p_graph, a_graph);
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
                accepted_requests_BM[(int)event.request_id/(num_requests/num_batch)]++;
            }
            else
            {
                blocked_requests++;
                blocked_requests_BM[(int)event.request_id/(num_requests/num_batch)]++;
                blocked_bandwidth += event.bandwidth;
                blocked_bandwidth_BM[(int)event.request_id/(num_requests/num_batch)] += event.bandwidth;
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
                    case LightPath::OTDM:
                        slot_st = lp->spectrum.slot_st;
                        slot_ed = lp->spectrum.slot_ed;

                        for(unsigned int node_i = 0; node_i < path.size() - 1; node_i++)
                        {
                            int from = path[node_i];
                            int to = path[node_i + 1];
                            Phy_link& link = p_graph.get_link(from, to);

                            for(int i = slot_st; i <= slot_ed; i++)
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
                    case LightPath::OFDM_WOB:
                    case LightPath::OFDM_WB:
                    case LightPath::OFDM:
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

                            for(int i = slot_st; i <= slot_ed; i++)
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
                    lp->available_bitrate += event.bandwidth;
                }
            }
            end_time = event.arrival_time;
        }
    }
    total_network_bandwidth = slot_capacity * num_slots * p_graph.link_list.size();
    print_result(traffic);
    return 0;
}

void construct_exist_path(Event& event, Phy_graph& p_graph, Aux_graph& a_graph)
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
        int n = lp->p_path.size() - 1;
        double weight = (double)n * (1 - eps) * lp->weight;

        for(unsigned int i = 0; i < lp->p_path.size(); i++)
        {
            int phy_node_id = lp->p_path[i];

            a_node = a_graph.get_adding_node(phy_node_id);
            d_node = a_graph.get_dropping_node(phy_node_id);

            t_node = a_graph.create_aux_node(phy_node_id, Aux_node::transmitting_node);

            lp->transmitting_node_list.push_back(t_node);

            r_node = a_graph.create_aux_node(phy_node_id, Aux_node::receiving_node);
            lp->receiving_node_list.push_back(r_node);

            aux_link = a_graph.create_aux_link(r_node, t_node, -weight, Aux_link::pass_through_link);
            lp->aux_link_list.push_back(aux_link);
            aux_link->light_path = lp;              // make aux_link track light path

            if(lp->transmitter_index[i] != -1)
            {
                aux_link = a_graph.create_aux_link(a_node, t_node, used_transceiver_weight, Aux_link::adding_link);
            }
            else if(p_graph.get_node(lp->p_path[i]).num_available_transmitter > 0)
            {
                aux_link = a_graph.create_aux_link(a_node, t_node, transceiver_weight, Aux_link::virtual_adding_link);
            }
            lp->aux_link_list.push_back(aux_link);
            aux_link->light_path = lp;              // make aux_link track light path


            if(lp->receiver_index[i] != -1)
            {
                aux_link = a_graph.create_aux_link(r_node, d_node, used_transceiver_weight, Aux_link::dropping_link);
            }
            else if(p_graph.get_node(lp->p_path[i]).num_available_receiver > 0)
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
            aux_link = a_graph.create_aux_link(t_node, r_node, weight, Aux_link::spectrum_link);
            // cout << "lp : " << lp << "  link: " << aux_link << "\n";
            lp->aux_link_list.push_back(aux_link);
            aux_link->light_path = lp;              // make aux_link track light path
        }
    }
}

void construct_candidate_path(Event& event, Phy_graph& p_graph, Aux_graph& a_graph)
{
    for(int source = 0; source < num_nodes; source++)
    {
        for(int destination = 0; destination < num_nodes; destination++)
        {
            if(source == destination)
            {
                continue;
            }

            if(enable_OTDM)
            {
                LightPath* OTDM_lp = get_best_OTDM_light_path(source, destination, event, p_graph);
                build_candidate_link(a_graph, OTDM_lp);
            }
            LightPath* OFDM_lp = get_best_OFDM_light_path(source, destination, event, p_graph);
            build_candidate_link(a_graph, OFDM_lp);
            LightPath* OFDM_WB_lp = get_best_OFDM_WB_light_path(source, destination, event, p_graph);
            build_candidate_link(a_graph, OFDM_WB_lp);
            LightPath* OFDM_WOB_lp = get_best_OFDM_WOB_light_path(source, destination, event, p_graph);
            build_candidate_link(a_graph, OFDM_WOB_lp);
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
    for(auto &lp : candidate_light_path_list)
    {
        for(auto &aux_link : lp->aux_link_list)
        {
            delete aux_link;
        }
        delete lp;
    }
    candidate_light_path_list.clear();
}

void build_candidate_link(Aux_graph& a_graph, LightPath* lpath)
{
    if(lpath == NULL)
    {
        return;
    }

    int source = lpath->p_path.front();
    int destination = lpath->p_path.back();

    Aux_node* v_t_node = NULL;
    Aux_node* v_r_node = NULL;

    switch (lpath->type)
    {
    case LightPath::OTDM:
        v_t_node = a_graph.get_OTDM_virtual_transmitting_node(source);
        v_r_node = a_graph.get_OTDM_virtual_receiving_node(destination);
        if(lpath->available_bitrate >= OTDM_threshold)
        {
            lpath->weight = lpath->weight * 0.1;
        }
        break;
    case LightPath::OFDM:
        v_t_node = a_graph.get_OFDM_virtual_transmitting_node(source);
        v_r_node = a_graph.get_OFDM_virtual_receiving_node(destination);
        break;
    case LightPath::OFDM_WB:
        v_t_node = a_graph.get_OFDM_WB_virtual_transmitting_node(source);
        v_r_node = a_graph.get_OFDM_WB_virtual_receiving_node(destination);
        break;
    case LightPath::OFDM_WOB:
        v_t_node = a_graph.get_OFDM_WOB_virtual_transmitting_node(source);
        v_r_node = a_graph.get_OFDM_WOB_virtual_receiving_node(destination);
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

void build_light_path(Phy_graph& p_graph, LightPath* candidate_path, Aux_node* aux_source, Aux_node* aux_destination, Event& event)
{
    Phy_node& src_node = p_graph.get_node(aux_source->phy_id);
    Phy_node& dst_node = p_graph.get_node(aux_destination->phy_id);
    LightPath* new_path = new LightPath();

    request2lightpath[event.request_id].push_back(new_path);
    new_path->requests.insert(event.request_id);
    new_path->modulation_level = candidate_path->modulation_level;
    new_path->available_bitrate = candidate_path->available_bitrate;
    new_path->weight = candidate_path->weight;
    new_path->spectrum = candidate_path->spectrum;
    new_path->p_path = candidate_path->p_path;
    new_path->type = candidate_path->type;


    int slot_st = new_path->spectrum.slot_st;
    int slot_ed = new_path->spectrum.slot_ed;
    Path& path = new_path->p_path;
    double used_bandwidth;
    double wasted_bandwidth;
    double guardband_bandwidth;
    double total_bandwidth;
    double num_links;

    switch(candidate_path->type)
    {
    case LightPath::OTDM:

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

            for(int i = slot_st + 1; i <= slot_ed - 1; i++)
            {
                link.slot[i] = 1;
            }
        }
        exist_OTDM_light_path_list.push_back(new_path);
        num_OTDM_lightpath_use++;
        num_OTDM_lightpath_use_BM[(int)event.request_id/(num_requests/num_batch)]++;

        num_links = 1.0 * new_path->p_path.size() - 1;
        used_bandwidth = num_links * event.bandwidth / new_path->modulation_level;
        wasted_bandwidth = 0;
        guardband_bandwidth = num_links * 2 * slot_capacity;
        total_bandwidth = used_bandwidth + wasted_bandwidth + guardband_bandwidth;
        total_bandwidth_utilization += total_bandwidth * event.holding_time;
        used_bandwidth_utilization += used_bandwidth * event.holding_time;
        wasted_bandwidth_utilization += wasted_bandwidth * event.holding_time;
        guardband_bandwidth_utilization += guardband_bandwidth * event.holding_time;
        break;

    case LightPath:: OFDM:
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
            for(int i = slot_st + 1; i <= slot_ed - 1; i++)
            {
                link.slot[i] = 1;
            }
        }
        exist_OFDM_light_path_list.push_back(new_path);
        num_OFDM_lightpath_use++;
        num_OFDM_lightpath_use_BM[(int)event.request_id/(num_requests/num_batch)]++;

        num_links = 1.0 * new_path->p_path.size() - 1;
        used_bandwidth = num_links * event.bandwidth / new_path->modulation_level;
        wasted_bandwidth = num_links * (slot_capacity - (used_bandwidth - slot_capacity * ((int)used_bandwidth / slot_capacity)));
        guardband_bandwidth = num_links * 2 * slot_capacity;
        total_bandwidth = used_bandwidth + wasted_bandwidth + guardband_bandwidth;
        total_bandwidth_utilization += total_bandwidth * event.holding_time;
        used_bandwidth_utilization += used_bandwidth * event.holding_time;
        wasted_bandwidth_utilization += wasted_bandwidth * event.holding_time;
        guardband_bandwidth_utilization += guardband_bandwidth * event.holding_time;
        break;

    case LightPath:: OFDM_WB:
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
            for(int i = slot_st + 1; i <= slot_ed - 1; i++)
            {
                link.slot[i] = 1;
            }
        }
        exist_OFDM_light_path_list.push_back(new_path);
        num_OFDM_lightpath_use++;
        num_OFDM_lightpath_use_BM[(int)event.request_id/(num_requests/num_batch)]++;

        num_links = 1.0 * new_path->p_path.size() - 1;
        used_bandwidth = num_links * event.bandwidth / new_path->modulation_level;
        wasted_bandwidth = num_links * (slot_capacity - (used_bandwidth - slot_capacity * ((int)used_bandwidth / slot_capacity)));
        guardband_bandwidth = num_links * slot_capacity;
        total_bandwidth = used_bandwidth + wasted_bandwidth + guardband_bandwidth;
        total_bandwidth_utilization += total_bandwidth * event.holding_time;
        used_bandwidth_utilization += used_bandwidth * event.holding_time;
        wasted_bandwidth_utilization += wasted_bandwidth * event.holding_time;
        guardband_bandwidth_utilization += guardband_bandwidth * event.holding_time;
        break;

    case LightPath:: OFDM_WOB:
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
            for(int i = slot_st + 1; i <= slot_ed - 1; i++)
            {
                link.slot[i] = 1;
            }
        }
        exist_OFDM_light_path_list.push_back(new_path);
        num_OFDM_lightpath_use++;
        num_OFDM_lightpath_use_BM[(int)event.request_id/(num_requests/num_batch)]++;

        num_links = 1.0 * new_path->p_path.size() - 1;
        used_bandwidth = num_links * event.bandwidth / new_path->modulation_level;
        wasted_bandwidth = num_links * (slot_capacity - (used_bandwidth - slot_capacity * ((int)used_bandwidth / slot_capacity)));
        guardband_bandwidth = num_links * slot_capacity;
        total_bandwidth = used_bandwidth + wasted_bandwidth + guardband_bandwidth;
        total_bandwidth_utilization += total_bandwidth * event.holding_time;
        used_bandwidth_utilization += used_bandwidth * event.holding_time;
        wasted_bandwidth_utilization += wasted_bandwidth * event.holding_time;
        guardband_bandwidth_utilization += guardband_bandwidth * event.holding_time;
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
    double used_bandwidth;
    double wasted_bandwidth;
    double guardband_bandwidth;
    double total_bandwidth;
    double num_links = 0;
    double num_links_a_lp;
    while(aux_node != aux_source)
    {
        switch(aux_link->type)
        {
        case Aux_link::candidate_link:
            build_light_path(p_graph, aux_link->light_path, aux_link->from, aux_link->to, event);
            break;
        case Aux_link::spectrum_link:
            if(aux_link->light_path->requests.find(event.request_id) == aux_link->light_path->requests.end())
            {
                request2lightpath[event.request_id].push_back(aux_link->light_path);
                aux_link->light_path->requests.insert(event.request_id);
                aux_link->light_path->available_bitrate -= event.bandwidth;
                num_OTDM_lightpath_use++;
                num_OTDM_lightpath_use_BM[(int)event.request_id/(num_requests/num_batch)]++;
            }
            num_links += 1;
            break;
        case Aux_link::grooming_link:
            num_OEO++;
            num_OEO_BM[(int)event.request_id/(num_requests/num_batch)]++;
            break;
        case Aux_link::adding_link:
            if(aux_link->light_path == NULL) // not aux_link for candidate light path layer
            {
                break;
            }
            if(aux_link->light_path->type == LightPath::OTDM) // aux_link to OTDM grooming layer
            {
                num_links = 0;
            }
            break;
        case Aux_link::dropping_link:
            if(aux_link->light_path == NULL) // not aux_link for candidate light path layer
            {
                break;
            }
            if(aux_link->light_path->type == LightPath::OTDM) // aux_link to OTDM grooming layer
            {
                num_links_a_lp = 1.0 * (aux_link->light_path->p_path.size() - 1);
                used_bandwidth = num_links * event.bandwidth / aux_link->light_path->modulation_level;
                wasted_bandwidth = num_links_a_lp * (event.bandwidth / aux_link->light_path->modulation_level) - used_bandwidth;
                guardband_bandwidth = 0;
                total_bandwidth = used_bandwidth + wasted_bandwidth + guardband_bandwidth;
                total_bandwidth_utilization += total_bandwidth * event.holding_time;
                used_bandwidth_utilization += used_bandwidth * event.holding_time;
                wasted_bandwidth_utilization += wasted_bandwidth * event.holding_time;
                guardband_bandwidth_utilization += guardband_bandwidth * event.holding_time;
            }
            break;
        case Aux_link::pass_through_link:
            break;
        case Aux_link::virtual_adding_link:
            if(aux_link->light_path == NULL) // not aux_link for candidate light path layer
            {
                break;
            }
            if(aux_link->light_path->type == LightPath::OTDM) // aux_link to OTDM grooming layer
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
                num_links = 0;
            }
            break;
        case Aux_link::virtual_dropping_link:
            if(aux_link->light_path == NULL) // not aux_link for candidate light path layer
            {
                break;
            }
            if(aux_link->light_path->type == LightPath::OTDM) // aux_link to OTDM grooming layer
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

                num_links_a_lp = 1.0 * (aux_link->light_path->p_path.size() - 1);
                used_bandwidth = num_links * event.bandwidth / aux_link->light_path->modulation_level;
                wasted_bandwidth = num_links_a_lp * (event.bandwidth / aux_link->light_path->modulation_level) - used_bandwidth;
                guardband_bandwidth = 0;
                total_bandwidth = used_bandwidth + wasted_bandwidth + guardband_bandwidth;
                total_bandwidth_utilization += total_bandwidth * event.holding_time;
                used_bandwidth_utilization += used_bandwidth * event.holding_time;
                wasted_bandwidth_utilization += wasted_bandwidth * event.holding_time;
                guardband_bandwidth_utilization += guardband_bandwidth * event.holding_time;
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
        while(i < num_slots)
        {
            i++;
            if(link.slot[i] == -1)
            {
                return i; // index of first free slot
            }
        }
        return num_slots + 1;
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
        int bound;
        bound = (slot_st - search_scope + 1 >= 0)? search_scope : slot_st + 1;
        for(int i = 1; i < bound; i++)
        {
            if(link.slot[slot_st - i] == 0)
            {
                distance += i;
                zone_clear = false;
                break;
            }
        }
        bound = (slot_ed + search_scope - 1 < num_slots)? search_scope : num_slots - slot_ed;
        for(int i = 1; i < bound; i++)
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
        return transceiver_slot_limit*4*hop_limit;
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
        if(slot_st > 0)
        {
            if(link.slot[slot_st - 1] == -1)
            {
                num_cut++;
            }
        }
        if(slot_ed < num_cut - 1)
        {
            if(link.slot[slot_ed + 1] == -1)
            {
                num_cut++;
            }
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
    return transceiver_slot_limit*4*hop_limit-reserved_coefficent * distance + cut_coeffcient * num_cut + align_coeffcient * num_align;
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
    while(slot_st + require_slots - 1 < num_slots)
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

LightPath* get_best_OTDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
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

    int modulation_level = 0;
    int available_bitrate = 0;
    list<CandidatePath>& c_path_list = p_graph.get_path_list(source, destination);
    Spectrum path_spectrum, best_path_spectrum;
    Path best_p_path;

    best_path_spectrum.slot_st = -1;
    best_path_spectrum.slot_ed = -1;
    best_path_spectrum.weight = DBL_MAX;

    for(auto &c_path : c_path_list)
    {
        int require_slots = num_guardband_slot * 2 + ceil(1.0 * event.bandwidth / c_path.modulation_level / slot_capacity);
        path_spectrum = find_best_spectrum(c_path.path, require_slots, p_graph);
        if(best_path_spectrum.weight > path_spectrum.weight){
            best_p_path = c_path.path;
            best_path_spectrum = path_spectrum;
            modulation_level = c_path.modulation_level;
            available_bitrate = (require_slots - num_guardband_slot * 2) * c_path.modulation_level * slot_capacity - event.bandwidth;
        }
    }
    if(best_path_spectrum.slot_st < 0){
        return NULL;
    }
    LightPath* path = new LightPath();
    path->type = LightPath::OTDM;
    path->p_path = best_p_path;
    path->modulation_level = modulation_level;
    path->available_bitrate = available_bitrate;
    path->weight = best_path_spectrum.weight;
    path->spectrum = best_path_spectrum;
    return path;
}

int get_available_OFDM_transceiver(vector<OFDMTransceiver>& transceivers)
{
    for(int i = 0; i < (int)transceivers.size(); i++)
    {
        if(transceivers[i].in_used == false)
        {
            return i;
        }
    }
    cout << "error can not find available transceiver\n";
    exit(1);
    return -1;
}

LightPath* get_best_OFDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
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

    int modulation_level = 0;
    int available_bitrate = 0;
    list<CandidatePath>& c_path_list = p_graph.get_path_list(source, destination);
    Spectrum path_spectrum, best_path_spectrum;
    Path best_p_path;

    best_path_spectrum.slot_st = -1;
    best_path_spectrum.slot_ed = -1;
    best_path_spectrum.weight = DBL_MAX;

    for(auto &c_path : c_path_list)
    {
        int require_slots = num_guardband_slot * 2 + ceil(1.0 * event.bandwidth / c_path.modulation_level / slot_capacity);
        path_spectrum = find_best_spectrum(c_path.path, require_slots, p_graph);
        if(best_path_spectrum.weight > path_spectrum.weight){
            best_p_path = c_path.path;
            best_path_spectrum = path_spectrum;
            modulation_level = c_path.modulation_level;
            available_bitrate = (require_slots - num_guardband_slot * 2) * c_path.modulation_level * slot_capacity - event.bandwidth;
        }
    }
    if(best_path_spectrum.slot_st < 0){
        return NULL;
    }
    LightPath* path = new LightPath();
    path->type = LightPath::OFDM;
    path->p_path = best_p_path;
    path->modulation_level = modulation_level;
    path->available_bitrate = available_bitrate;
    path->weight = best_path_spectrum.weight;
    path->spectrum = best_path_spectrum;
    return path;
}

LightPath* get_best_OFDM_WOB_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
{
    Phy_node& src_node = p_graph.get_node(source);

    Spectrum path_spectrum, best_path_spectrum;
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

        // find the spectrum over the exisiting LightPath
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

    if(best_path_spectrum.slot_st < 0)
    {
        return NULL;
    }

    int require_slots = ceil(1.0 * event.bandwidth / best_existing_lightpath->modulation_level / slot_capacity);
    int available_bitrate = (require_slots - num_guardband_slot * 2) * best_existing_lightpath->modulation_level * slot_capacity - event.bandwidth;

    LightPath* path = new LightPath();

    path->type = LightPath::OFDM_WOB;
    path->modulation_level = best_existing_lightpath->modulation_level;
    path->available_bitrate = available_bitrate;
    path->p_path = best_existing_lightpath->p_path;
    path->transmitter_index = best_existing_lightpath->transmitter_index;
    path->receiver_index = best_existing_lightpath->receiver_index;
    path->weight = best_path_spectrum.weight;
    path->spectrum = best_path_spectrum;

    return path;
}

LightPath* get_best_OFDM_WB_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
{
    Phy_node& src_node = p_graph.get_node(source);
    Phy_node& dst_node = p_graph.get_node(destination);

    LightPath* best_existing_lightpath = NULL;
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
                num_hops = c_path.path.size() - 1;
            }
            else
            {
                num_hops = lp->p_path.size() - 1;
            }

            split_node_i = num_hops;

            for(unsigned int node_i = 0; node_i <= num_hops; node_i++)
            {
                if(c_path.path[node_i] != lp->p_path[node_i])
                {
                    split_node_i = node_i - 1;
                    break;
                }
            }

            if(split_node_i < 1)
            {
                continue;
            }

            if(split_node_i == (int)lp->p_path.size()-1) // use OFDM WOB instead
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

            // spectrum over the exisiting LightPath
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

            // spectrum under the exisiting LightPath
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

    int require_slots = ceil(1.0 * event.bandwidth / best_existing_lightpath->modulation_level / slot_capacity);
    int available_bitrate = (require_slots - num_guardband_slot * 2) * best_existing_lightpath->modulation_level * slot_capacity - event.bandwidth;

    LightPath* path = new LightPath();
    path->type = LightPath::OFDM_WB;
    path->p_path = best_p_path;
    path->modulation_level = best_existing_lightpath->modulation_level;
    path->available_bitrate = available_bitrate;
    path->weight = best_path_spectrum.weight;
    path->spectrum = best_path_spectrum;
    path->transmitter_index = best_existing_lightpath->transmitter_index;
    path->receiver_index = best_existing_lightpath->receiver_index;
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

void print_result(Traffic traffic)
{
    cout << "Output result\n";
    ostringstream filename;

    filename << "result_";

    if(enable_OTDM)
    {
        filename << "OTDM_";
    }
    else
    {
        filename << "SLICE_";
    }

    filename << traffic_lambda << "_" << num_requests << "_" << num_transceiver << "_" << graph_file;

    ofstream ofs (filename.str(), ofstream::out);

    double num_OEO_s2 = 0;
    double num_OFDM_lightpath_use_s2 = 0;
    double num_OTDM_lightpath_use_s2 = 0;
    for(int i = 0; i < num_batch; i++)
    {
        num_OEO_BM[i] = (double)num_OEO_BM[i]/accepted_requests_BM[i] - (double)num_OEO/accepted_requests;
        num_OFDM_lightpath_use_BM[i] = (double)num_OFDM_lightpath_use_BM[i]/accepted_requests_BM[i] - (double)num_OFDM_lightpath_use/accepted_requests;
        num_OTDM_lightpath_use_BM[i] = (double)num_OTDM_lightpath_use_BM[i]/accepted_requests_BM[i] - (double)num_OTDM_lightpath_use/accepted_requests;

        num_OEO_BM[i] = num_OEO_BM[i]*num_OEO_BM[i];
        num_OFDM_lightpath_use_BM[i] = num_OFDM_lightpath_use_BM[i]*num_OFDM_lightpath_use_BM[i];
        num_OTDM_lightpath_use_BM[i] = num_OTDM_lightpath_use_BM[i]*num_OTDM_lightpath_use_BM[i];

        num_OEO_s2 += num_OEO_BM[i];
        num_OFDM_lightpath_use_s2 += num_OFDM_lightpath_use_BM[i];
        num_OTDM_lightpath_use_s2 += num_OFDM_lightpath_use_BM[i];
    }
    num_OEO_s2 /= (num_batch-1);
    num_OFDM_lightpath_use_s2 /= (num_batch-1);
    num_OTDM_lightpath_use_s2 /= (num_batch-1);

    num_OEO_s2 /= num_batch;
    num_OFDM_lightpath_use_s2 /= num_batch;
    num_OTDM_lightpath_use_s2 /= num_batch;

    num_OEO_s2 = sqrt(num_OEO_s2);
    num_OFDM_lightpath_use_s2 = sqrt(num_OFDM_lightpath_use_s2);
    num_OTDM_lightpath_use_s2 = sqrt(num_OTDM_lightpath_use_s2);

    double p_hat;

    p_hat = (double)blocked_bandwidth/total_bandwidth;
    p_hat = (double)p_hat*(1-p_hat)/total_bandwidth;
    ofs << "Bandwidth Blocking Ratio: "<<(double)blocked_bandwidth/total_bandwidth<< " +- " << (double)1.960*sqrt(p_hat)<< endl;
    ofs << "Blocked Requests: "<<blocked_requests<<endl;
    ofs << "Accepted Requests: "<<accepted_requests<<endl;
    p_hat = (double)blocked_requests/num_requests;
    p_hat = (double)p_hat*(1-p_hat)/num_requests;
    ofs << "Blocking Probability: "<<(double)blocked_requests/num_requests << " +- " << (double)1.960*sqrt(p_hat)<< endl;
    ofs << "Total Number of OEO: "<< num_OEO << endl;
    ofs << "Average OEO when Request Succeed: "<<(double)num_OEO/accepted_requests<<" +- t* "<<(double)num_OEO_s2<< endl;
    ofs << "Number of OFDM lightpath use: "<< num_OFDM_lightpath_use << endl;
    ofs << "Average OFDM lightpath use when Request Succeed: "<<(double)num_OFDM_lightpath_use/accepted_requests<<" +- t* "<<(double)num_OFDM_lightpath_use_s2<< endl;
    ofs << "Number of OTDM lightpath use: "<< num_OTDM_lightpath_use << endl;
    ofs << "Average OTDM lightpath use when Request Succeed: "<<(double)num_OTDM_lightpath_use/accepted_requests<<" +- t* "<<(double)num_OTDM_lightpath_use_s2<< endl;
    ofs << "Bandwidth Utilization Factor: "<<(double)total_bandwidth_utilization/(total_network_bandwidth*end_time)<<endl;
    ofs << "Used Bandwidth Utilization Factor: "<<(double)used_bandwidth_utilization/(total_network_bandwidth*end_time)<<endl;
    ofs << "Wasted Bandwidth Utilization Factor: "<<(double)wasted_bandwidth_utilization/(total_network_bandwidth*end_time)<<endl;
    ofs << "Guardband Bandwidth Utilization Factor: "<<(double)guardband_bandwidth_utilization/(total_network_bandwidth*end_time)<<endl;
    ofs << "Load:"<< traffic_lambda << endl << endl;
    ofs << "Number of OTDM transceiver per degree: "<< num_OTDM_transceiver << endl;
    ofs << "Number of OFDM transceiver per degree: "<< num_OFDM_transceiver << endl;
    ofs << "Number of OC1 requests: " << traffic.num_OC1_request << endl;
    ofs << "Number of OC3 requests: " << traffic.num_OC3_request << endl;
    ofs << "Number of OC9 requests: " << traffic.num_OC9_request << endl;
    ofs << "Number of OC12 requests: " << traffic.num_OC12_request << endl;
    ofs << "Number of OC18 requests: " << traffic.num_OC18_request << endl;
    ofs << "Number of OC24 requests: " << traffic.num_OC24_request << endl;
    ofs << "Number of OC36 requests: " << traffic.num_OC36_request << endl;
    ofs << "Number of OC48 requests: " << traffic.num_OC48_request << endl;
    ofs << "Number of OC192 requests: " << traffic.num_OC192_request << endl;
    ofs << "Number of OC768 requests: " << traffic.num_OC768_request << endl;
    ofs << "Number of OC3072 requests: " << traffic.num_OC3072_request << endl;


    ofs << "Elapsed time: " << (double) ( clock() - start_clk ) / CLOCKS_PER_SEC << " seconds" << endl;
    ofs << "Time spending for graph construction: "<<clk_construction<<" seconds" << endl;
    ofs << "Time spending for path finding: "<<clk_finding<<" seconds" << endl;
    ofs << "Time spending for path parsing: "<<clk_parsing<<" seconds" << endl<<endl;

    ofs.close();
}
