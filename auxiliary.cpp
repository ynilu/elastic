#include "auxiliary.h"

using namespace std;

Aux_node::Aux_node(int id)
{
    this -> id = id;
    first_in = NULL;
    last_in = NULL;
    first_out = NULL;
    last_out = NULL;
}

Aux_node::~Aux_node()
{
    Aux_link *current;
    Aux_link *next;

    // remove all the in links
    current = this -> first_in;
    while(current != NULL)
    {
        next = current -> next_same_to;
        delete current;
        current = next;
    }

    // remove all the out links
    current = this -> first_out;
    while(current != NULL)
    {
        next = current -> next_same_from;
        delete current;
        current = next;
    }
}

Aux_link::Aux_link(Aux_node* from, Aux_node* to, float cost)
{
    this -> cost = cost;

    this -> from = from;
    this -> to = to;

    // same from list
    Aux_link *from_last_out = from -> last_out;
    if(from_last_out == NULL) // list is empty;
    {
        this -> prev_same_from = NULL;
        this -> next_same_from = NULL;
        from -> first_out = this;
        from -> last_out = this;
    }
    else
    {
        this -> prev_same_from = from_last_out;
        this -> next_same_from = NULL;
        from_last_out -> next_same_from = this;
        from -> last_out = this;
    }

    // same to list
    Aux_link *to_last_in = to -> last_in;
    if(to_last_in == NULL) // list is empty;
    {
        this -> prev_same_to = NULL;
        this -> next_same_to = NULL;
        to -> first_in = this;
        to -> last_in = this;
    }
    else
    {
        this -> prev_same_to = to_last_in;
        this -> next_same_to = NULL;
        to_last_in -> next_same_to = this;
        to -> last_in = this;
    }
}

Aux_link::~Aux_link()
{
    Aux_link *prev;
    Aux_link *next;

    // same_from list
    prev = this -> prev_same_from;
    next = this -> next_same_from;

    if(prev == NULL && next == NULL) // link is the only element in same_from list
    {
        from -> first_out = NULL;
        from -> last_out = NULL;
    }
    else if(prev == NULL) // link is the first in same_from list
    {
        from -> first_out = next;
        next -> prev_same_from = NULL;
    }
    else if(next == NULL) // link is the last in same_from list
    {
        from -> last_out = prev;
        prev -> next_same_from = NULL;
    }
    else
    {
        prev -> next_same_from = next;
        next -> prev_same_from = prev;
    }

    // same_to list
    prev = this -> prev_same_to;
    next = this -> next_same_to;

    if(prev == NULL && next == NULL) // link is the only element in same_to list
    {
        to -> first_in = NULL;
        to -> last_in = NULL;
    }
    else if(prev == NULL) // link is the first in same_to list
    {
        to -> first_in = next;
        next -> prev_same_to = NULL;
    }
    else if(next == NULL) // link is the last in same_to list
    {
        to -> last_in = prev;
        prev -> next_same_to = NULL;
    }
    else
    {
        prev -> next_same_to = next;
        next -> prev_same_to = prev;
    }
}
