#include "delta.h"


void delta::symbolicEncoding(DES* G, bdd addPred){
    for(auto& trList: transitionList){
        bdd encPred = bddfalse;
        for(auto& tr: trList.second)
            encPred |= tr->encode(G);
        trList.first->setNSigma(addPred & encPred);
    }
}
