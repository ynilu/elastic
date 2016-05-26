#ifndef AUXILIARY_H
#define AUXILIARY_H

#include <iostream>

class Aux_link;

class Aux_node
{
public:
    int id;
    Aux_link *first_in, *last_in, *first_out, *last_out;

    Aux_node(int id);
    virtual ~Aux_node();
};

class Aux_link
{
public:
    Aux_node *from, *to;
    Aux_link *prev_same_from, *next_same_from, *prev_same_to, *next_same_to;
    float cost;

    Aux_link(Aux_node* from, Aux_node* to, float cost);
    virtual ~Aux_link();
};

class Aux_graph
{
public:

    Aux_graph();
    virtual ~Aux_graph();
};
#endif /* AUXILIARY_H */
