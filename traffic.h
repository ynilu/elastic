#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <iostream>
#include <set>

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
    set<int> destination;
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

    float random_number( int seed );

    double get_interarrival_time( float mean, int seed );

    long long nextrand( long long &seed );
};


#endif /* TRAFFIC_H */
