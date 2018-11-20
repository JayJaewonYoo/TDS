#ifndef EVENT_H
#define EVENT_H

#include <iostream>
#include <memory>
#include <set>
#include <fdd.h>
#include <vector>
#include <algorithm>

using namespace std;

class DES;

class event{
public:
    // Constructors
    event();
    event(string sigma);
    event(string sigma, bool _ctrb);
    //
    string getName();
    string printName();
    bool isControllable();
    void addContainer(DES* G);
    void insertSrc(int i);
    void insertDst(int i);
    void setNSigma(bdd P);
    bdd  getNSigma();
    void setVarSets();
    bdd delta(bdd pred);
    bdd gamma(bdd pred);
    bdd getFSigma();
    void setFSigma(bdd P);
    bdd handleIllegalPredicate(bdd& P);
private:
    string name;
    bool   ctrb; // event is controllable if "ctrb = true"; default value is "true";
    set<DES*> container; // set of all DESs that contain a transition via this event;
    set<int> srcVars;// set of correpsonding source fdd variables;
    set<int> dstVars;// set of corresponding destrination fdd variables;
    bdd nSigma;//encoded transition under this event;
    bdd fSigma;//synthesized control functions;
    bdd setOfSrcVars;// \mathbf{v}'_{\sigma,S}
    bdd setOfDstVars;// \mathbf{V}_{\sigma,T}
    bddPair* srcToPsrc;// v_{\sigma,S}\rightarrow v'_{\sigma,S}
    bddPair* psrcToSrc;// v'_{\sigma,S}\rightarrow v_{\sigma,S}
};


typedef unique_ptr<event> ePtr;

class eCmp{
public:
    bool operator()(const ePtr& e1, const ePtr& e2)const;
};

#endif // EVENT_H
