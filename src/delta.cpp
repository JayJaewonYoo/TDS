#include "delta.h"

void delta::addTransition(transition* t){
    tPtr temp(t);
    transitionList[t->getEvent()].push_back(move(temp));
}

void delta::printTransitionStructure(ostream& out){
    for(auto& sigma:transitionList){
        for(auto& transition:sigma.second){
            transition->printTransition(out);
        }
    }
}

void delta::symbolicEncoding(DES* G, bdd addPred){
    for(auto& trList: transitionList){
        bdd encPred = bddfalse;
        for(auto& tr: trList.second)
            encPred |= tr->encode(G);
        trList.first->setNSigma(addPred & encPred);
    }
}
