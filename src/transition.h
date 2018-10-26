#ifndef TRANSITION_H
#define TRANSITION_H

#include <iostream>
#include <memory>
#include <istream>
#include <set>

#include <fdd.h>

using namespace std;

class event;
class DES;


class badSrcDES{};// Used to throw exceptions when non-simple source is added!
class badDstDES{};// Used to throw exceptions when non-simple destination is added!


class transition{
private:
    set<DES*> src;//set of source simple DESs - Note: source state is always a simple DES;
    event* sigma;
    set<DES*> dst;//set of destination simple DESs
public:
    transition():sigma{nullptr}{}
    transition(event* _sigma):sigma{_sigma}{}
    event* setEvent(event* _sigma, DES* G);
    void setEvent(event* _sigma){ sigma = _sigma;}
    event* getEvent(){return sigma;}
    void addSrc(istream& is, DES* G, int numOfSrcs);
    void addSrc(DES* sDES);
    void addDst(istream& is, DES* G, int numOfDsts);
    void addDst(DES* dDES);


    void printTransition(ostream& out);

    bdd encode(DES* G);


};

typedef unique_ptr<transition> tPtr;

#endif // TRANSITION_H
