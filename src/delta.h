#ifndef DELTA_H
#define DELTA_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include "event.h"
#include "transition.h"


using namespace std;

// This class implements the transition structure of an XOR TDS.


class delta{
public:
    void addTransition(transition* t);
    void printTransitionStructure(ostream& out);
    // The following method encodes all transitions inside
    // DES G. "addPred" is G's encoding predicate, used for
    // encoding transitions in lower levels.
    void symbolicEncoding(DES* G, bdd addPred = bddtrue);
private:
    unordered_map<event*, vector<tPtr>> transitionList;
};


typedef unique_ptr<delta> dPtr;


#endif // DELTA_H
