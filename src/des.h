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
protected:
    string name;
    DES* parent;//pointer to parent DES
    int numOfComponents;//number of components

    int fddDomain;//id of the assigned variable
    int fddState;//value of the parent variable
    bdd addPred;//bdd used for encoding the path to the root in a TDS

public:
    //constructors
    DES(): name{""}, parent{nullptr}, addPred{bddtrue}{}
    DES(string _name): name{_name}, parent{nullptr},
        numOfComponents{0}, addPred{bddtrue}{}
    DES(string _name, DES* _parent, int noc): name{_name},
        parent{_parent}, numOfComponents{noc}, addPred{bddtrue}{}
    DES(string _name, DES* _parent, int noc, int _fddState): name{_name},
        parent{_parent}, numOfComponents{noc}, fddState{_fddState}{
        if (_parent ==  nullptr)
            addPred = bddtrue;
        else if (_parent->getType() == DEStype::SYNC)
            addPred = _parent->getAddPred();
        else
            addPred = _parent->getAddPred() &
                    fdd_ithvar(_parent->getfddDomain(), _fddState);
    }

    void setPar(DES* _parent){parent = _parent;}
    DES* getPar(){return parent;}
    void setAddPred(bdd P){addPred = P;}
    bdd  getAddPred(){return addPred;}

    virtual void setfddDomain(int i){
        cerr << "Error: setfddDomain  called on " << this->getName() << "!" << endl;
        abort();
    }
    virtual int getfddDomain(){
        cerr << "Error: getfddDomain  called on " << this->getName() << "!" << endl;
        abort();
    }

    virtual bool isInitial(){
        cerr << "Error: isInitial  called on " << this->getName() << "!" << endl;
        abort();
    }

    virtual bool isMarker(){
        cerr << "Error: isMarker  called on " << this->getName() << "!" << endl;
        abort();
    }
    virtual void setfddState(int i){ fddState = i;}
    int getfddState(){return fddState;}

    string getName()const{return name;}
    virtual string printName()const {return name;}
    virtual DES* addComponent(DES* T){
        cerr << "Error: addComponent called on " << this->getName() << "." << endl;
        abort();
    }
    virtual void addEvent(event* sigma){
        cerr << "Error: addEvent called on " << this->getName() << "." << endl;
        abort();
    }
    virtual void addDelta(delta* d){
        cerr << "Error: addDelta called on " << this->getName() << "." << endl;
        abort();
    }
    virtual bdd theta(DES* G, bool source = false, event* sigma = nullptr){
        cerr << "Error: theta called on " << this->getName() << "." << endl;
        abort();
    }
    virtual DES* findComponent(string _name){
        return nullptr;
    }
    virtual int getCompSize(){return 0;}
    virtual DEStype getType()const = 0;
    // printing functions
    virtual ostream& print(ostream& out)const = 0;
    friend ostream& operator<<(ostream& out, const DES& T) {return T.print(out);}
};

typedef unique_ptr<DES> DESPtr;
class DESCmp{
public:
    bool operator()(const DESPtr& p1, const DESPtr& p2){
        return p1->getName() < p2->getName();
    }
};


class simpleDES: public DES{
private:
    bool initial;// default = false,
    bool marker;// default  = false,
public:
//    simpleDES(string _name):DES(_name,nullptr, 0){}
//    simpleDES(string _name, DES* _parent):DES(_name, _parent, 0){}
    simpleDES(string _name, DES* _parent, int _fddState,
              bool _initial = false, bool _marker = false)
        :DES(_name, _parent, 0, _fddState),
          initial{_initial}, marker{_marker} {}

    bool isInitial(){return initial;}
    bool isMarker(){return marker;}
    string printName()const {
        string retS = name;
        if (marker) retS += "*";
        if (initial) retS += "(i)";
        return retS;
    }
    DEStype getType()const{return DEStype::SIMPLE;}
    ostream& print(ostream& out) const;
};


class xorDES: public DES{
private:
    set<DESPtr, DESCmp> components;// set of components; they can be simple, xor, or sync DESs.
    set<event*> Sigma;// alphabet of event labels
    dPtr transitionStructure;// delta
public:
    xorDES(string _name, DES* _parent, int noc, int _fddState): DES(_name,_parent, noc, _fddState) {
        // add new fdd variable
        // fddDomain - 1 will be the primed variable
        vector<int> temp{noc, noc};
        fdd_extdomain(&temp[0], temp.size());
        this->setfddDomain( fdd_domainnum() - 1);
        fdd_hook_suorce.push_back(this->getName() + "'");
        fdd_hook_suorce.push_back(this->getName());
    }
    DEStype getType()const{return DEStype::XOR;}
    void setfddDomain(int i){ fddDomain = i;}
    int getfddDomain(){return fddDomain;}
    int getCompSize(){return components.size();}
    // adding components: if T is already in the compenent set, a pointer to T will be returned; otherwise
    // a unique pointer of T is created and added the component set and a pointer to this newly added component will
    //be returned.
    DES* addComponent(DES* T){
        DESPtr temp(T);
        auto it = components.insert(move(temp));
        return it.first->get();
    }
    DES* findComponent(string G){
        for(auto& c:components){
            if( c->getName() == G)
                return c.get();
            else{
                DES* comp = c->findComponent(G);
                if(comp != nullptr)
                    return comp;
            }
        }
        return nullptr;
    }
    void addEvent(event* sigma){
        Sigma.insert(sigma);
        //check posssibility of checking local coupling???
    }

    void addDelta(delta* d){
        transitionStructure.reset(d);
    }
    //theta_G(q): if source == true, then primed variables are used to encode!
    bdd theta(DES* G, bool source = false, event* sigma = nullptr);

    // printting function: Prints components and transitions.
    ostream& print(ostream& out) const;
private:

};


class syncDES: public DES{
private:
    set<DESPtr, DESCmp> components;
public:
    syncDES(string _name, DES* _parent, int noc , int _fddState)
        :DES(_name, _parent, noc, _fddState){}

    int getCompSize(){return components.size();}
    DES* addComponent(DES* T){
        DESPtr temp(T);
        auto it = components.insert(move(temp));
        return it.first->get();
    }
    DES* findComponent(string G){
        for(auto& c:components){
            if( c->getName() == G)
                return c.get();
            else{
                DES* comp = c->findComponent(G);
                if(comp != nullptr)
                    return comp;
            }
        }
        return nullptr;
    }
    DEStype getType()const{return DEStype::SYNC;}
    ostream& print(ostream& out) const;

};

#endif // DES_H
