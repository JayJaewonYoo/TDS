#ifndef DELTA_H
#define DELTA_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include "event.h"
#include "transition.h"


using namespace std;

// This class implements the transition structure of an xor TDS.


class delta{
private:
    unordered_map<event*, vector<tPtr>> transitionList;
public:
    void addTransition(transition* t){
        tPtr temp(t);
        transitionList[t->getEvent()].push_back(move(temp));
    }

    void printTransitionStructure(ostream& out){
        for(auto& sigma:transitionList){
            for(auto& transition:sigma.second){
                transition->printTransition(out);
            }
        }
    }
    // The following method encodes all transitions inside
    // DES G. "addPred" is G's encoding predicate, used for
    // encoding transitions in lower levels.
    void symbolicEncoding(DES* G, bdd addPred = bddtrue);
};


typedef unique_ptr<delta> dPtr;


#endif // DELTA_H
