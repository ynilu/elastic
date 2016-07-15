#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <iostream>
#include <set>
#include <vector>
#include <fstream>
#include <list>
#include <cmath>

struct Traffic_info
{
    char* source_file;
    char* traffic_file;
    int num_nodes;
    int num_requests;
    int traffic_lambda;
    int traffic_mu;
    float unicast_percentage;
    long long aTime_seed;
    long long hTime_seed;
    long long s_seed;
    long long d_seed;
    long long numD_seed;
    long long b_seed;
    int OC1_share;
    int OC3_share;
    int OC9_share;
    int OC12_share;
    int OC18_share;
    int OC24_share;
    int OC36_share;
    int OC48_share;
    int OC192_share;
    int OC768_share;
    int OC3072_share;
};

class Request
{
public:
    int request_id;
    int source;
    std::set<int> destination;
    int num_dest;
    int bandwidth;
    float arrival_time;
    float holding_time;

};

class Event
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
    bool operator <(const Event&) const;

    Event();
    virtual ~Event();
};


class Traffic
{
public:
    std::vector< std::vector<int> > total_dest_count;
    std::vector< std::vector<double> > traffic_matrix;
    std::vector<double> source_matrix;
    std::vector<int> num_dest_matrix;
    std::list<Event> event_list;
    std::vector<Request> request_list;

    int num_nodes;
    int num_requests;
    int total_OCx_share;
    double OC1_ratio;
    double OC3_ratio;
    double OC9_ratio;
    double OC12_ratio;
    double OC18_ratio;
    double OC24_ratio;
    double OC36_ratio;
    double OC48_ratio;
    double OC192_ratio;
    double OC768_ratio;
    double OC3072_ratio;
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

    bool empty();
    Event next_event();
    void delete_event(int request_id);

    int generate_num_dest(int max_num_dest);
    int generate_source();
    int generate_destination(int source);
    int generate_bandwidth();

    float random_number( int seed );
    double get_interarrival_time( float mean, int seed );
    long long nextrand( long long &seed );

    Traffic(Traffic_info& t_info);
    virtual ~Traffic();

};


#endif /* TRAFFIC_H */
