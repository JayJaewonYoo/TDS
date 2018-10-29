#include "des.h"

vector<string> fdd_hook_suorce;

void  printhandler(FILE *o, int var){
    fprintf(o, "%s", fdd_hook_suorce[var].c_str());
}

// DES class methods:
DES::DES(): name{""}, parent{nullptr}, addPred{bddtrue}{}
DES::DES(string _name): name{_name}, parent{nullptr},
    numOfComponents{0}, addPred{bddtrue}{}
DES::DES(string _name, DES* _parent, int noc): name{_name},
    parent{_parent}, numOfComponents{noc}, addPred{bddtrue}{}
DES::DES(string _name, DES* _parent, int noc, int _fddState): name{_name},
    parent{_parent}, numOfComponents{noc}, fddState{_fddState}{
    if (_parent ==  nullptr)
        addPred = bddtrue;
    else if (_parent->getType() == DEStype::SYNC)
        addPred = _parent->getAddPred();
    else
        addPred = _parent->getAddPred() &
                fdd_ithvar(_parent->getfddDomain(), _fddState);
}

void DES::setPar(DES* _parent){parent = _parent;}
DES* DES::getPar(){return parent;}
void DES::setAddPred(bdd P){addPred = P;}
bdd  DES::getAddPred(){return addPred;}

void DES::setfddState(int i){ fddState = i;}
int DES::getfddState(){return fddState;}

string DES::getName()const{return name;}
string DES::printName()const {return name;}
int DES::getCompSize(){return 0;}

void DES::setfddDomain(int i){
    cerr << "Error: setfddDomain  called on " << this->getName() << "!" << endl;
    abort();
}
int DES::getfddDomain(){
    cerr << "Error: getfddDomain  called on " << this->getName() << "!" << endl;
    abort();
}
DES* DES::addComponent(DES* T){
    cerr << "Error: addComponent called on " << this->getName() << "." << endl;
    abort();
}
void DES::addEvent(event* sigma){
    cerr << "Error: addEvent called on " << this->getName() << "." << endl;
    abort();
}
void DES::addDelta(delta* d){
    cerr << "Error: addDelta called on " << this->getName() << "." << endl;
    abort();
}
bdd DES::theta(DES* G, bool source, event* sigma){
    cerr << "Error: theta called on " << this->getName() << "." << endl;
    abort();
}
bdd DES::initialPredicate(){
    cerr << "Error: initialPredicate called on " << this->getName() << "." << endl;
    abort();
}
bdd DES::markerPredicate(){
    cerr << "Error: markerPredicate called on " << this->getName() << "." << endl;
    abort();
}
bool DES::isInitial(){return false;}
bool DES::isMarker(){return false;}

DES* DES::findComponent(string _name){
    return nullptr;
}
ostream& operator<<(ostream& out, const DES& T) {return T.print(out);}

bool DESCmp::operator()(const DESPtr& p1, const DESPtr& p2){
    return p1->getName() < p2->getName();
}

//simpleDES class methods:
simpleDES::simpleDES(string _name, DES* _parent, int _fddState,
          bool _initial, bool _marker)
    :DES(_name, _parent, 0, _fddState),
      initial{_initial}, marker{_marker} {}

bool simpleDES::isInitial(){return initial;}
bool simpleDES::isMarker(){return marker;}
string simpleDES::printName()const {
    string retS = name;
    if (marker) retS += "*";
    if (initial) retS += "(i)";
    return retS;
}
DEStype simpleDES::getType()const{return DEStype::SIMPLE;}

ostream& simpleDES::print(ostream& out) const{
    out << this->getName();
    return out;
}


//xorDES class methods
xorDES::xorDES(string _name, DES* _parent, int noc, int _fddState):
    DES(_name,_parent, noc, _fddState) {
    // add new fdd variable
    // fddDomain - 1 will be the primed variable
    vector<int> temp{noc, noc};
    fdd_extdomain(&temp[0], temp.size());
    this->setfddDomain( fdd_domainnum() - 1);
    fdd_hook_suorce.push_back(this->getName() + "'");
    fdd_hook_suorce.push_back(this->getName());
}
DEStype xorDES::getType()const{return DEStype::XOR;}
void xorDES::setfddDomain(int i){ fddDomain = i;}
int xorDES::getfddDomain(){return fddDomain;}
int xorDES::getCompSize(){return components.size();}

DES* xorDES::addComponent(DES* T){
    DESPtr temp(T);
    auto it = components.insert(move(temp));
    return it.first->get();
}
DES* xorDES::findComponent(string G){
    if (this->getName() == G)
        return this;
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
void xorDES::addEvent(event* sigma){
    Sigma.insert(sigma);
    //check posssibility of checking local coupling???
}

void xorDES::addDelta(delta* d){
    transitionStructure.reset(d);
}
bdd xorDES::initialPredicate(){
    for(auto& c:components){
        if (c->isInitial())
            return (this->theta(c.get()) & addPred);
        else if (c->getType()!= DEStype::SIMPLE){
            bdd pred = c->initialPredicate();
            if (pred != bddfalse)
                return pred;
        }
    }
    return bddfalse;
}
bdd xorDES::markerPredicate(){
    bdd pred = bddfalse;
    for(auto& c:components){
        if (c->isMarker())
            pred |= this->theta(c.get());
        else if (c->getType()!= DEStype::SIMPLE){
            pred |= c->markerPredicate();
        }
    }
    return pred & addPred;
}
bdd xorDES::theta(DES* G, bool source, event* sigma){
    bdd retBDD = bddtrue;
    DES* currentSys = G;
    while(currentSys != this){
        if ( currentSys == nullptr){
            cerr << "Error in encoding " << G->getName()
                 << " inside " << this->getName() << "!" << endl;
            abort();
        }
        else if ( syncDES* pt = dynamic_cast<syncDES*>(currentSys->getPar())){
            currentSys = currentSys->getPar();
            continue;
        }
        else if (source){
            retBDD &= fdd_ithvar(currentSys->getPar()->getfddDomain() - 1,
                                 currentSys->getfddState());
            if (sigma != nullptr)
                   sigma->insertSrc(currentSys->getPar()->getfddDomain());
        }
        else{
            retBDD &= fdd_ithvar(currentSys->getPar()->getfddDomain(),
                             currentSys->getfddState());
            if (sigma != nullptr)
                   sigma->insertDst(currentSys->getPar()->getfddDomain());
        }
        currentSys = currentSys->getPar();
    }
    return retBDD;
}
ostream& xorDES::print(ostream& out) const{
    //first, output xor or sync components.
    for(auto& c:components){
        simpleDES* pt = dynamic_cast<simpleDES*>(c.get());
        if(pt == nullptr)
            out << *c;
    }
    //second, its own components are outputed.
    out << this->getName() <<"\nComponents are: " << endl;
    for(auto& c:components)
        out << c->getfddState() << ":" <<c->printName() << " ";
    out << "\nEvents are: " << endl;
    for(auto& sigma:Sigma)
        out << sigma->printName() << " ";
    out << "\nTransitions are: " << endl;
    //output transitions
    if(transitionStructure != nullptr)
        transitionStructure->printTransitionStructure(out);
    out << "--------**------------" << endl;
    return out;
}


//syncDES class methods
syncDES::syncDES(string _name, DES* _parent, int noc , int _fddState)
    :DES(_name, _parent, noc, _fddState){}

int syncDES::getCompSize(){return components.size();}
DES* syncDES::addComponent(DES* T){
    DESPtr temp(T);
    auto it = components.insert(move(temp));
    return it.first->get();
}
DES* syncDES::findComponent(string G){
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
bdd syncDES::initialPredicate(){
    bdd pred = bddtrue;
    for(auto& c:components)
        pred &= c->initialPredicate();
    return pred;
}
bdd syncDES::markerPredicate(){
    bdd pred = bddtrue;
    for(auto& c:components)
        pred &= c->markerPredicate();
    return pred;
}
DEStype syncDES::getType()const{return DEStype::SYNC;}
ostream& syncDES::print(ostream& out) const{
    //First, output xor or sync components.
    for(auto& c:components){
        simpleDES* pt = dynamic_cast<simpleDES*>(c.get());
        if(pt == nullptr)
            out << *c;
    }
    //Second, its own components are outputed.
    out << this->getName() << "\nComponents are: " << endl;
    for(auto& c:components)
        out << c->getfddState() << ":" <<c->printName() << " ";
    out << "\n--------**------------" << endl;
    return out;
}

