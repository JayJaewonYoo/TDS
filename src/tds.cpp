#include "tds.h"

TDS::TDS(string fileName, string filePath){
    root.reset(inputReader(fileName, filePath));
    initialSet = false;
    supC2P = bddtrue;
}


DES* TDS::inputReader(string filePath, string fileName, DES* par){
    ifstream is(filePath + "/" + fileName + ".txt");
    if(is.fail()){
        cerr << "Error: Input file " << filePath + "/" + fileName + ".txt"
             << " not found!" << endl;
        abort();
    }
    cout << "Inupt file" << filePath + "/" + fileName + ".txt" << " opened!" << endl;

    string name, type;
    int size, tsize;// component size and number of transitions, tsize = 0 in case of sync TDSs
    is >> name >> type >> size >> tsize;
    if(is.fail()){
        cerr << "Error: Input file " << filePath + "/" + fileName + ".txt"
             << " name read fail!" << endl;
        abort();
    }

    DES* newDES;
    int fddState = (par)? par->getCompSize(): 0;
    if ( type == "xor" ){
        newDES = new xorDES(name,par,size,fddState);
        readComponents(filePath,is,newDES,size);
        readTransitions(is,newDES,tsize);
    }
    else if ( type == "sync" ){
        newDES = new syncDES(name,par,size,fddState);
        readComponents(filePath,is,newDES,size);
    }
    else {
        cerr << "Error: Unkown type " << type << " in " <<
                filePath + "/" + name + ".txt" << endl;
        abort();
    }
    cout << "Inupt file" << filePath + "/" + name + ".txt" << " read!" << endl;
    return newDES;
}


void TDS::readComponents(string filePath, ifstream& is, DES* parent, int size){
    cout << "Reading components of " <<
            filePath + "/" + parent->getName() + ".txt!" << endl;
    for( int i = 0; i < size; ++i){
        string name, type;
        is >> name >> type;
        DES* newDES;
        if ( type == "simple"){
            bool init, marker;
            is >> init >> marker;
            if (is.fail()){
                cerr << "Error: initial and marker bits are not set for simple DES "
                     << name << " in " << filePath + "/" + parent->getName() + ".txt!" << endl;
                abort();
            }
            newDES = new simpleDES(name,parent, parent->getCompSize(),init,marker);
        }
        else if (type == "xor" || type == "sync")
           newDES = inputReader(filePath, name,parent);
        else {
            cerr << "Error: Unkown type " << type << " in " <<
                    filePath + "/" + name + ".txt" << endl;
            abort();
        }
        parent->addComponent(newDES);
    }
    cout << "Components of " <<
            filePath + "/" + parent->getName() + ".txt" << " read!" << endl;
}

void TDS::readTransitions(ifstream& is, DES* G, int tsize){
    cout << "Reading transitions of " <<
            G->getName() << "!" << endl;
    delta* transitionStructure = new delta();
    for(int i = 0; i < tsize; ++i){
        transition* newTransition = new transition();
        int numOfSrcs, numOfDsts;
        is >> numOfSrcs >> numOfDsts;

        newTransition->addSrc(is,G,numOfSrcs);
        newTransition->setEvent(this->addEvent(is), G);
        newTransition->addDst(is,G,numOfDsts);
        transitionStructure->addTransition(newTransition);
    }
    transitionStructure->symbolicEncoding(G, G->getAddPred());
    G->addDelta(transitionStructure);
    cout << "Transitions  of " <<
            G->getName() << " read!" << endl;
}
event* TDS::addEvent(istream& is){
    string e;
    bool ctrb;
    is >> e >> ctrb;
    if ( Sigma[e] == nullptr)
        Sigma[e].reset(new event(e,ctrb));
    return Sigma[e].get();
}
// open-loop run of the system
void TDS::setInitials(){
    P0 = root->initialPredicate();
    Pm = root->markerPredicate();
    for(auto& sigma:Sigma)
        sigma.second->setVarSets();
    initialSet = true;
}

void TDS::run(){
    cout << "Open loop simulation: " << endl;
    if(!initialSet)
        setInitials();
    vector<bdd> visitedStates;
    visitedStates.push_back(P0);
    int numOfTransitions = 0;
    runRec(P0,visitedStates,0, numOfTransitions);
    cout << "States: " << visitedStates.size() <<
            ", Transitions: " << numOfTransitions << endl;
}
void TDS::runRec(bdd current, vector<bdd>& visitedStates, int currIndex, int& numOfTransitions){
    cout << currIndex << ":"; fdd_printset(current);
    if ( (current & Pm) != bddfalse)
        cout << "*";
    cout << endl;
    for(auto& sigma:Sigma){
        bdd next = sigma.second->delta(current);
        if ( next != bddfalse ){
            numOfTransitions++;
            cout << currIndex << "-Via: " << sigma.first <<endl;
            auto it = find_if(visitedStates.begin(), visitedStates.end(),[&](auto& P){
                return P == next;
            });
            int nextIndex = distance(visitedStates.begin(),it);
            if (it == visitedStates.end()){
                visitedStates.push_back(next);
                runRec(next,visitedStates,nextIndex, numOfTransitions);
            }
            else{
                cout << nextIndex << ": "; fdd_printset(next);cout << endl;
            }
         }
    }
}

//specification given in dijuntive form
//predicate corresponding to illegal set of states
bdd TDS::readSpecFile(string filePath){
    ifstream input(filePath + "/spec.txt");
    if (input.fail()){
        cerr << "Error: spec file not found!" << endl;
        abort();
    }
    int numOfLiterals;
    input >> numOfLiterals;
    bdd predicate = bddfalse;
    while(numOfLiterals){
        int sizeOfLiteral;
        input >> sizeOfLiteral;
        bdd literal = bddtrue;
        while(sizeOfLiteral){
            string var, value;
            input >> var >> value;
            DES* D1 = root->findComponent(var);
            if (D1 == nullptr){
                cerr << "Error: DES " << var << " not found in spec file!" << endl;
                abort();
            }
            DES* D2 = root->findComponent(value);
            if (D2 == nullptr){
                cerr << "Error: DES " << value << " not found in spec file!" << endl;
                abort();
            }
            if (D1->getType() != DEStype::XOR){
                cerr << "Error: DES " << var << " is not an XOR product DES!" << endl;
                abort();
            }
            literal &= fdd_ithvar(D1->getfddDomain(), D2->getfddState());
            literal &= D1->getAddPred();
            sizeOfLiteral--;
        }
        predicate |= literal;
        numOfLiterals--;
    }
    return bdd_not(predicate);
}
void TDS::supcon(string filePath){
    if(!initialSet)
        setInitials();
    bdd P = readSpecFile(filePath);
    //fdd_printset(P); cout << endl;
    createSupC2P(P);
    //fdd_printset(supC2P); cout << endl;
    for(auto& sigma:Sigma)
        sigma.second->setFSigma(supC2P);
}
void TDS::createSupC2P(bdd& P){
    bdd newBDD1 = P;
    bdd newBDD2 = P;
    do{
       newBDD1 = newBDD2;
       bdd temp = supCP(newBDD1);
       newBDD2 = P & CR(temp);
    }while( newBDD1 != newBDD2);
    supC2P = newBDD2;
}
bdd TDS::CR(bdd& P){
    bdd newBDD1 = P & Pm;
    bdd newBDD2 = P & Pm;
    do{
        newBDD1 = newBDD2;
        for(auto& sigma:Sigma){
            newBDD2 |= (P & sigma.second->gamma(newBDD1));
        }
    }while( newBDD1 != newBDD2);
    return newBDD2;
}
bdd TDS::supCP(bdd& P){
    bdd temp = bdd_not(P);
    return bdd_not(Bracket(temp));
}

bdd TDS::Bracket(bdd& P){
    bdd newBDD1 = P;
    bdd newBDD2 = P;
    do{
        newBDD1 = newBDD2;
        for(auto& sigma:Sigma){
            if(!sigma.second->isControllable()){
                newBDD2 |= sigma.second->gamma(newBDD1);
            }
        }
    }while( newBDD2 != newBDD1);
    return newBDD2;
}
//
void TDS::runUnderControl(){
    cout << "Closed loop simulation: " << endl;
    if(!initialSet)
        setInitials();
    vector<bdd> visitedStates;
    if ((P0 & supC2P) == bddfalse){
        cout << "Closed behavior is empty!" << endl;
        return;
    }
    visitedStates.push_back(P0);
    int numOfTransitions = 0;
    runUnderControlRec(P0,visitedStates,0, numOfTransitions);
    cout << "States: " << visitedStates.size() <<
            ", Transitions: " << numOfTransitions << endl;
}

void TDS::runUnderControlRec(bdd current, vector<bdd>& visitedStates, int currIndex, int& numOfTransitions){
    cout << currIndex << ":"; fdd_printset(current);
    if ( (current & Pm) != bddfalse)
        cout << "*";
    cout << endl;

    for(auto& sigma:Sigma){
        if((sigma.second->getFSigma() & current) != bddfalse){
            bdd next = sigma.second->delta(current);
            if ( next != bddfalse ){
                numOfTransitions++;
                cout << currIndex << "-Via: " << sigma.first <<endl;
                auto it = find_if(visitedStates.begin(), visitedStates.end(),[&](auto& P){
                    return P == next;
                });
                int nextIndex = distance(visitedStates.begin(),it);

                if (it == visitedStates.end()){
                    visitedStates.push_back(next);
                    runUnderControlRec(next,visitedStates,nextIndex, numOfTransitions);
                }
                else {
                    cout << nextIndex << ": "; fdd_printset(next);cout << endl;
                }
            }
        }
    }
}
void TDS::print(){
    cout << *root;
}
