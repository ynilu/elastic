#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <iostream>

class Spectrum
{
public:
    int slot_st;   // start number of occupied slots
    int slot_ed;   // end number of occupied slots
    double weight;

    Spectrum();
    virtual ~Spectrum();
};

#endif /* SPECTRUM_H */
