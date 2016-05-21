#include "traffic.h"

void Generate_traffic()
{
//	double event_time = 0;
    double arrival_time = 0;
    for( unsigned int i = 0 ; i < REQUEST_NUMBER ; ++i )
    {
        event arrival;
        event departure;

        arrival.Arrival_or_Departure = 0;
        departure.Arrival_or_Departure = 1;
        arrival.event_index = i;
        departure.event_index = i;
        //		int temp;
        bool dest[ NODE_NUMBER ];
        for( int i=0 ; i<NODE_NUMBER ; i++ )
        {
            dest[i] = 0;
            //multicast
            //arrival.dest[i] = 0;
            //departure.dest[i] =0;
            arrival.dest.clear();
            departure.dest.clear();
        }
        // decide destination set
        int source;
        if(READ_TRAFFIC_FILE)
        {
            float p = random_number(nextrand(s_seed));
            for(int i=0;i<NODE_NUMBER;i++){
                p = p - (float)source_matrix[i];
                if(p<=0){
                    source = i;
                    arrival.source = source;
                    departure.source = source;
                    break;
                }
            }
        }else{
            source = (int) ( NODE_NUMBER*random_number(nextrand(s_seed)) );
            arrival.source = source;
            departure.source = source;
        }
        int mcast_size;
        float temp_m = random_number(nextrand(numD_seed));
        if(READ_TRAFFIC_FILE)
        {
            if(temp_m<=unicast_percentage){
                mcast_size= 0;
            }else{
                mcast_size = (int) ( mcast_size_matrix[source]*random_number(nextrand(numD_seed)) );
            }
        }else{
            if(temp_m<=unicast_percentage){
                mcast_size= 0;
            }else{
                mcast_size = (int) ( (NODE_NUMBER-1)*temp_m);
            }
        }
//		if( mcast_size==0 )
        mcast_size += 1;

        arrival.num_d = mcast_size;
        departure.num_d = mcast_size;
        //cout<<"s:"<<source<<" m:"<<mcast_size<<" d:";
        int count = 0;
        bool repeat = 0;
        int temp_d;
        while( count<mcast_size )
        {
            repeat = 0;
            if(READ_TRAFFIC_FILE)
            {
                float p = random_number(nextrand(s_seed));
                for(int i=0;i<NODE_NUMBER;i++){
                    p = p - (float)traffic_matrix[source][i];
                    if(p<=0){
                        temp_d = i;
                        break;
                    }
                }
            }else{
                temp_d = (int) ( NODE_NUMBER*random_number(nextrand(d_seed)) );
            }
            if( temp_d!=source )
            {
                for( int j=0 ; j<NODE_NUMBER ; ++j )
                {
                    if( dest[temp_d]==1 )
                    {
                        repeat = 1;
                        break;
                    }
                }
                if( repeat==0 )
                {
                    dest[temp_d] = 1;
                    //multicast
                    //arrival.dest[temp_d] = 1;
                    //departure.dest[temp_d] = 1;
                    arrival.dest.insert(temp_d);
                    departure.dest.insert(temp_d);
                    total_dest_count[source][temp_d]++;
                    count++;
                }
            }
        }

        int bandwidth;
        float temp_b;
        temp_b = random_number( nextrand(b_seed) );
        if( temp_b<=0.25 )
        {
            bandwidth = 1;//1
            total_bandwidth_count[0]++;
        }
        else if( 0.25<temp_b && temp_b<=0.5)
        {
            bandwidth = 3;//3
            total_bandwidth_count[1]++;
        }
        else if( 0.5<temp_b && temp_b<=0.75 )
        {
            bandwidth = 12;//12
            total_bandwidth_count[2]++;
        }
        else if( 0.75<temp_b && temp_b<=1 )
        {
            bandwidth = 48;
            total_bandwidth_count[3]++;
        }
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
    return;
}

int event::generate_source(int s_seed, int NODE_NUMBER)
{
    float p = random_number(nextrand(s_seed));
    for(int i=0;i<NODE_NUMBER;i++){
        p = p - (float)source_matrix[i];
        if(p<=0){
            return i;
        }
    }
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
    return (seed/2147483646.0);
}

// ------- Get the arrival time or the holding time -------
double traffic::get_interarrival_time( float mean, int seed )
{
    return (-1/mean)*log(1.0-(double)seed/2147483646.0);
}

// ------- Random Number Generator -------
long long traffic::nextrand( long long &seed )
{
    seed = (16807*seed)%(2147483647);
    return seed;
}
