#include "traffic.h"

using namespace std;

Traffic::Traffic(Traffic_info& t_info)
{
    num_nodes = t_info.num_nodes;
    num_requests = t_info.num_requests;
    bandwidth_max = t_info.bandwidth_max;
    bandwidth_min = t_info.bandwidth_min;
    traffic_lambda = t_info.traffic_lambda;
    traffic_mu = t_info.traffic_mu;
    unicast_percentage = t_info.unicast_percentage;
    aTime_seed = t_info.aTime_seed;
    hTime_seed = t_info.hTime_seed;
    s_seed = t_info.s_seed;
    d_seed = t_info.d_seed;
    numD_seed = t_info.numD_seed;
    b_seed = t_info.b_seed;

    // resize the containers for the data read from file
    source_matrix.resize(num_nodes);
    num_dest_matrix.resize(num_nodes);
    traffic_matrix.resize(num_nodes);
    for(auto &element : traffic_matrix)
    {
        element.resize(num_nodes);
    }

    total_dest_count.resize(num_nodes);
    for(auto &element : total_dest_count)
    {
        element.resize(num_nodes);
    }

    read_source_file(t_info.source_file);
    read_traffic_file(t_info.traffic_file);

    generate_traffic();

}

Traffic::~Traffic()
{

}
void Traffic::read_source_file(char* source_file)
{
    fstream fs;

    fs.open(source_file, ios::in);
    if(!fs)
    {
        cerr << "Can't open file\"" << source_file << "\"\n";
        cerr << "Please give source file, program aborting...\n";
        exit(1);
    }

    int node_a, max_num_dest;
    double source_probility;
    while(fs >> node_a >> source_probility >> max_num_dest)
    {
        node_a--;
        source_matrix[node_a] = source_probility;
        num_dest_matrix[node_a] = max_num_dest;
    }

    fs.close();

}

void Traffic::read_traffic_file(char* traffic_file)
{
    fstream fs;

    fs.open(traffic_file, ios::in);
    if(!fs)
    {
        cerr << "Can't open file\"" << traffic_file << "\"\n";
        cerr << "Please give Traffic file, program aborting...\n";
        exit(1);
    }

    int node_a, node_b;
    double traffic_probility;
    while(fs >> node_a >> node_b >> traffic_probility)
    {
        node_a--;
        node_b--;
        traffic_matrix[node_a][node_b] = traffic_probility;
    }

    fs.close();

}

void Traffic::generate_traffic()
{
    double arrival_time = 0;
    for( int i = 0 ; i < num_requests ; ++i )
    {
        Event arrival;
        Event departure;

        arrival.type = Event::arrival;
        departure.type = Event::departure;
        arrival.request_id = i;
        departure.request_id = i;

        arrival.destination.clear();
        departure.destination.clear();

        // decide source
        int source = generate_source();
        arrival.source = source;
        departure.source = source;

        // decide destination set
        int num_dest = generate_num_dest(source_matrix[source]);
        arrival.num_dest = num_dest;
        departure.num_dest = num_dest;

        int count = 0;
        int temp_d;

        // gernerate all destinations (can be optimized)
        while( count < num_dest )
        {
            temp_d = generate_destination(source);

            if( temp_d != source )
            {
                if( arrival.destination.insert(temp_d).second == true ) // if a new element was inserted
                {
                    departure.destination.insert(temp_d);
                    total_dest_count[source][temp_d]++;
                    count++;
                }
            }
        }

        int bandwidth = generate_bandwidth();

        arrival.bandwidth = bandwidth;
        departure.bandwidth = bandwidth;

        arrival_time += get_interarrival_time( traffic_lambda, nextrand(aTime_seed) );
        double holding_time;
        holding_time = get_interarrival_time( traffic_mu, nextrand(hTime_seed) );

        arrival.arrival_time = arrival_time;
        arrival.holding_time = holding_time;

        event_list.push_back(arrival);

        departure.arrival_time = arrival_time + holding_time;
        departure.holding_time = 0;

        event_list.push_back(departure);
    }
    event_list.sort();
}

int Traffic::generate_num_dest(int max_num_dest)
{
    int num_dest;
    float temp_m = random_number(nextrand(numD_seed));
    if(temp_m <= unicast_percentage){
        num_dest = 1;
    }else{
        num_dest = (int) ( (max_num_dest - 1) * random_number(nextrand(numD_seed)) );
        num_dest += 2;
        //num_dest range : [2,max_num_dest]
    }
    return num_dest;
}

int Traffic::generate_source()
{
    float p = random_number(nextrand(s_seed));
    int i;
    for(i = 0; i < num_nodes; i++){
        p = p - (float)source_matrix[i];
        if(p <= 0){
            return i;
        }
    }
    return i;
}

int Traffic::generate_destination(int source)
{
    float p = random_number(nextrand(s_seed));
    int i;
    for(i = 0;i < num_nodes; i++){
        p = p - (float)traffic_matrix[source][i];
        if(p <= 0){
            return i;
        }
    }
    return i;
}

int Traffic::generate_bandwidth()
{
    int bandwidth;
    int num_points = bandwidth_max - bandwidth_min + 1;
    bandwidth = (int) (num_points * random_number( nextrand(b_seed) ));
    bandwidth += bandwidth_min;
    return bandwidth;
}

bool Event::operator <(const Event& a) const
{
    if( this -> arrival_time < a.arrival_time )
    {
        return true;
    }
    return false;
}

// ------- Create a random number between 0 and 1 -------
float Traffic::random_number( int seed )
{
    return (seed/2147483647.0);
}

// ------- Get the arrival time or the holding time -------
double Traffic::get_interarrival_time( float mean, int seed )
{
    return (-1/mean)*log(1.0-(double)seed/2147483646.0);
}

// ------- Random Number Generator -------
long long Traffic::nextrand( long long& seed )
{
    seed = (16807*seed)%(2147483647);
    return seed;
}

Event::Event()
{

}

Event::~Event()
{

}
