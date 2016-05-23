#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <iostream>
#include <set>
#include <vector>
#include <fstream>
#include <list>

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
    bool operator<(event);
    event();
    virtual ~event();
};


class traffic
{
public:
    traffic();
    virtual ~traffic();

    std::vector< std::vector<int> > total_dest_count;
    std::vector< std::vector<double> > traffic_matrix;
    std::vector<double> source_matrix;
    std::vector<int> num_dest_matrix;
    std::list<event> event_list;

    void read_source_file(char* source);
    void read_traffic_file(char* traffic);
    int generate_num_dest(int& numD_seed, int max_num_dest, float unicast_percentage);
    int generate_source(int& s_seed, int NODE_NUMBER, std::vector<double>& source_matrix);
    int generate_destination(int& s_seed, int NODE_NUMBER, std::vector<double>& traffic_matrix);
    int generate_bandwidth(int& b_seed, int min, int max);

    float random_number( int seed );
    double get_interarrival_time( float mean, int seed );
    long long nextrand( long long &seed );
};


#endif /* TRAFFIC_H */
