#include "des.h"

vector<string> fdd_hook_suorce;

void  printhandler(FILE *o, int var){
    fprintf(o, "%s", fdd_hook_suorce[var].c_str());
}


ostream& simpleDES::print(ostream& out) const{
    out << this->getName();
    return out;
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
                   sigma->insertSrc(currentSys->getPar()->getfddDomain() - 1);
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

