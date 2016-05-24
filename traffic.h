#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <iostream>
#include <set>
#include <vector>
#include <fstream>
#include <list>
#include <cmath>

struct traffic_info
{
    char* source_file;
    char* traffic_file;
    int num_nodes;
    int num_requests;
    int bandwidth_max;
    int bandwidth_min;
    int traffic_lambda;
    int traffic_mu;
    float unicast_percentage;
    long long aTime_seed;
    long long hTime_seed;
    long long s_seed;
    long long d_seed;
    long long numD_seed;
    long long b_seed;
};

class event
{
public:
    enum event_type{
        arrival,
        departure
    };
    event_type type;
    int request_id;
    int source;
    std::set<int> destination;
    int num_dest;
    int bandwidth;
    float arrival_time;
    float holding_time;
    bool operator <(const event&) const;

    event();
    virtual ~event();
};


class traffic
{
public:
    traffic(traffic_info& t_info);
    virtual ~traffic();

    std::vector< std::vector<int> > total_dest_count;
    std::vector< std::vector<double> > traffic_matrix;
    std::vector<double> source_matrix;
    std::vector<int> num_dest_matrix;
    std::list<event> event_list;

    int num_nodes;
    int num_requests;
    int bandwidth_max;
    int bandwidth_min;
    int traffic_lambda;
    int traffic_mu;
    float unicast_percentage;
    long long aTime_seed;
    long long hTime_seed;
    long long s_seed;
    long long d_seed;
    long long numD_seed;
    long long b_seed;

    void read_source_file(char* source_file);
    void read_traffic_file(char* traffic_file);

    void generate_traffic();

    int generate_num_dest(int max_num_dest);
    int generate_source();
    int generate_destination(int source);
    int generate_bandwidth();

    float random_number( int seed );
    double get_interarrival_time( float mean, int seed );
    long long nextrand( long long &seed );
};


#endif /* TRAFFIC_H */
