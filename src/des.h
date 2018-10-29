#ifndef DES_H
#define DES_H

#include <iostream>
#include <memory>
#include <set>
#include <algorithm>
#include "event.h"
#include "transition.h"
#include "delta.h"
#include <fstream>
#include <fdd.h>


using namespace std;

extern vector<string> fdd_hook_suorce;
void  printhandler(FILE *o, int var);

enum class DEStype{SIMPLE, XOR, SYNC};


class DES{
public:
    //constructors
    DES();
    DES(string _name);
    DES(string _name, DES* _parent, int noc);
    DES(string _name, DES* _parent, int noc, int _fddState);

    void setfddState(int i);
    int getfddState();

    string getName()const;
    virtual string printName()const;
    virtual int getCompSize();
    virtual DEStype getType()const = 0;

    void setPar(DES* _parent);
    DES* getPar();
    void setAddPred(bdd P);
    bdd  getAddPred();



    virtual void setfddDomain(int i);
    virtual int getfddDomain();
    virtual DES* addComponent(DES* T);
    virtual void addEvent(event* sigma);
    virtual void addDelta(delta* d);
    virtual bdd theta(DES* G, bool source = false, event* sigma = nullptr);
    virtual bdd initialPredicate();
    virtual bdd markerPredicate();
    virtual bool isInitial();
    virtual bool isMarker();

    virtual DES* findComponent(string _name);


    // printing functions
    virtual ostream& print(ostream& out)const = 0;
    friend ostream& operator<<(ostream& out, const DES& T);

protected:
    string name;
    DES* parent;//pointer to parent DES
    int numOfComponents;//number of components

    int fddDomain;//id of the assigned variable
    int fddState;//value of the parent variable
    bdd addPred;//bdd used for encoding the path to the root in a TDS
};


typedef unique_ptr<DES> DESPtr;
class DESCmp{
public:
    bool operator()(const DESPtr& p1, const DESPtr& p2);
};


class simpleDES: public DES{
public:
    simpleDES(string _name, DES* _parent, int _fddState,
              bool _initial = false, bool _marker = false);
    bool isInitial();
    bool isMarker();
    string printName()const;
    DEStype getType()const;
    ostream& print(ostream& out) const;
private:
    bool initial;// default = false,
    bool marker;// default  = false,
};


class xorDES: public DES{
public:
    xorDES(string _name, DES* _parent, int noc, int _fddState);
    DEStype getType()const;
    void setfddDomain(int i);
    int getfddDomain();
    int getCompSize();
    // adding components: if T is already in the compenent
    //set, a pointer to T will be returned; otherwise
    // a unique pointer of T is created and added to the component
    //set and a pointer to this newly added component will
    //be returned.
    DES* addComponent(DES* T);
    DES* findComponent(string G);
    void addEvent(event* sigma);

    void addDelta(delta* d);
    bdd initialPredicate();
    bdd markerPredicate();
    //theta_G(q): if source == true, then primed variables are used to encode!
    bdd theta(DES* G, bool source = false, event* sigma = nullptr);
    // printting function: Prints components and transitions.
    ostream& print(ostream& out) const;
private:
    set<DESPtr, DESCmp> components;// set of components; they can be simple, xor, or sync DESs.
    set<event*> Sigma;// alphabet of event labels
    dPtr transitionStructure;// delta
};


class syncDES: public DES{
public:
    syncDES(string _name, DES* _parent, int noc , int _fddState);

    int getCompSize();
    DES* addComponent(DES* T);
    DES* findComponent(string G);
    bdd initialPredicate();
    bdd markerPredicate();
    DEStype getType()const;
    ostream& print(ostream& out) const;
private:
    set<DESPtr, DESCmp> components;
};

#endif // DES_H
