#include "transition.h"
#include "des.h"
#include "event.h"

event* transition::setEvent(event* _sigma, DES* G){
    G->addEvent(_sigma);
    this->setEvent(_sigma);
    return _sigma;
}

void transition::addSrc(istream& is, DES* G, int numOfSrcs){
    for(int j = 0; j < numOfSrcs; ++j){
        string source;
        is >> source;
        DES* src = G->findComponent(source);
        if( src == nullptr){
            cerr << "Error: source DES " << source
                 << "not found in " << G->getName() << endl;
            abort();
        }
        this->addSrc(src);
    }
}
void transition::addSrc(DES* sDES){
    if(simpleDES* s = dynamic_cast<simpleDES*>(sDES)){
        src.insert(sDES);
        return;
    }
    else throw badSrcDES();
}

void transition::addDst(istream& is, DES* G, int numOfDsts){
    for(int j = 0; j < numOfDsts; ++j){
        string destination;
        is >> destination;
        DES* dst = G->findComponent(destination);
        if ( dst == nullptr){
            cerr << "Error: destination DES " << destination
                 << "not found in " << G->getName() << endl;
            abort();
        }
        this->addDst(dst);
    }
}
void transition::addDst(DES* dDES){
    if(simpleDES* s = dynamic_cast<simpleDES*>(dDES)){
        dst.insert(dDES);
        return;
    }
    else throw badDstDES();
}


void transition::printTransition(ostream& out){
    out << "from: (";
    for(auto& source:src)
        out << source->getName() << ",";
    out << "\b) via: " << sigma->printName() << " to: (";
    for(auto& destin:dst)
        out << destin->getName() << ",";
    out << "\b)" << endl;
    return;
}


bdd transition::encode(DES* G){
    bdd srcBDD = bddtrue;
    bdd dstBDD = bddtrue;
    for(auto& s:src)
        srcBDD &= G->theta(s,true, sigma);
    for(auto& d:dst)
        dstBDD &= G->theta(d,false,sigma);
    return srcBDD & dstBDD;
}
