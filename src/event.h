#ifndef EVENT_H
#define EVENT_H

#include <iostream>
#include <memory>
#include <set>
#include <fdd.h>

using namespace std;

class DES;

class event{
private:
    string name;
    bool   ctrb; // event is controllable if "ctrb = true"; default value is "true";
    set<DES*> container; // set of all DESs that contain a transition via this event;
    set<int> srcVars;// set of correpsonding source fdd variables;
    set<int> dstVars;// set of corresponding destrination fdd variables;
    bdd nSigma;//encoded transition under this event;
public:
    // Constructors
    event(){}
    event(string sigma): name{sigma}, ctrb{true}, nSigma{bddtrue}{}
    event(string sigma, bool _ctrb): name{sigma},
        ctrb{_ctrb}, nSigma{bddtrue}{}
    //
    string getName(){return name;}
    string printName(){
        string ctrb = (this->isControllable())? "(con)":"(unc)";
        return this->getName()+ctrb;
    }
    bool isControllable(){return ctrb;}
    void addContainer(DES* G){
        container.insert(G);
    }

    void insertSrc(int i){srcVars.insert(i);}
    void insertDst(int i){dstVars.insert(i);}

    void setNSigma(bdd P){nSigma &= P;}
    bdd  getNSigma(){return nSigma;}
};


typedef unique_ptr<event> ePtr;

class eCmp{
public:
    bool operator()(const ePtr& e1, const ePtr& e2)const{
        return e1->getName() < e2->getName();
    }
};

#endif // EVENT_H
