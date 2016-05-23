#include "traffic.h"

using namespace std;

traffic::traffic()
{
    source_matrix.resize(number_of_nodes);
    num_dest_matrix.resize(number_of_nodes);
    traffic_matrix.resize(number_of_nodes);

    for(auto &element : traffic_matrix)
    {
        element.resize(number_of_nodes);
    }
}

void traffic::read_source_file(char* source_file)
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

void traffic::read_traffic_file(char* traffic_file)
{
    fstream fs;

    fs.open(traffic_file, ios::in);
    if(!fs)
    {
        cerr << "Can't open file\"" << traffic_file << "\"\n";
        cerr << "Please give traffic file, program aborting...\n";
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
void Generate_traffic()
{
    double arrival_time = 0;
    for( unsigned int i = 0 ; i < REQUEST_NUMBER ; ++i )
    {
        event arrival;
        event departure;

        arrival.type = event::arrival;
        departure.type = event::departure;
        arrival.request_id = i;
        departure.request_id = i;

        arrival.dest.clear();
        departure.dest.clear();

        // decide source
        int source = generate_source(s_seed);
        arrival.source = source;
        departure.source = source;

        // decide destination set
        int num_dest = generate_num_dest();
        arrival.num_dest = num_dest;
        departure.num_dest = num_dest;

        int count = 0;
        int temp_d;

        // May be optimized
        while( count < num_dest )
        {
            temp_d = generate_destination(s_seed, NODE_NUMBER, );

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

        int bandwidth = generate_bandwidth(b_seed, b_min, b_max);

        arrival.bandwidth = bandwidth;
        departure.bandwidth = bandwidth;
        total_bandwidth += bandwidth;

        arrival_time += get_interarrival_time( TRAFFIC_LAMBDA , nextrand(aTime_seed) );
        double holding_time;
        holding_time = get_interarrival_time( TRAFFIC_MU , nextrand(hTime_seed) );

        arrival.arrival_time = arrival_time;
        arrival.holding_time = holding_time;

        event_list.push_back(arrival);

        departure.arrival_time = arrival_time + holding_time;
        departure.holding_time = 0;

        event_list.push_back(departure);
    }
    event_list.sort();
}

int traffic::generate_num_dest(int& numD_seed, int max_num_dest, float unicast_percentage)
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

int traffic::generate_source(int& s_seed, int NODE_NUMBER, vector<double>& source_matrix)
{
    float p = random_number(nextrand(s_seed));
    for(int i = 0;i < NODE_NUMBER; i++){
        p = p - (float)source_matrix[i];
        if(p <= 0){
            return i;
        }
    }
}

int traffic::generate_destination(int& s_seed, int NODE_NUMBER, vector<double>& traffic_matrix)
{
    float p = random_number(nextrand(s_seed));
    for(int i = 0;i < NODE_NUMBER; i++){
        p = p - (float)traffic_matrix[i];
        if(p <= 0){
            return i;
        }
    }
}

int traffic::generate_bandwidth(int& b_seed, int min, int max)
{
    int bandwidth;
    int num_points = max - min + 1;
    bandwidth = (int) (num_points * random_number( nextrand(b_seed) ));
    bandwidth += min;
    return bandwidth;
}

bool event::operator <(event a)
{
    if( arrival_time < a.arrival_time )
    {
        return 1;
    }
    return 0;
}

// ------- Create a random number between 0 and 1 -------
float traffic::random_number( int seed )
{
    return (seed/2147483647.0);
}

// ------- Get the arrival time or the holding time -------
double traffic::get_interarrival_time( float mean, int seed )
{
    return (-1/mean)*log(1.0-(double)seed/2147483646.0);
}

// ------- Random Number Generator -------
long long traffic::nextrand( long long& seed )
{
    seed = (16807*seed)%(2147483647);
    return seed;
}
