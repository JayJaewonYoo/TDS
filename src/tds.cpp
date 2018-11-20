#include "tds.h"
#include "io.h"
#include "spec.h"

TDS::TDS(string fileName, string filePath){
    root.reset(IO::inputReader(fileName, filePath,nullptr, this));
    initialSet = false;
    supC2P = bddtrue;
}
DES* TDS::findComponent(string G){
    return root->findComponent(G);
}
event* TDS::findEvent(string s){
    auto it = Sigma.find(s);
    if (it == Sigma.end())
        return nullptr;
    else return it->second.get();
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

void TDS::run(string G){
    if(!initialSet)
        setInitials();
    vector<bdd> visitedStates;
    visitedStates.push_back(P0);
    int numOfTransitions = 0;
    bdd setOfVars = bddtrue;
    bool localSimulation = false;
    if (!G.empty()){
        DES* localRoot = root->findComponent(G);
        if (localRoot == nullptr){
            cerr << "Error: Local root " << G << " not found!" << endl;
            abort();
        }
        int localFddIndex = localRoot->getfddDomain();
        for(int i = 1; i < fdd_domainnum(); i+=2){
            if (i != localFddIndex )
                setOfVars &= fdd_ithset(i);
        }
        localSimulation = true;
        cout << "Open-loop behavior under the component " << localRoot->getName() << endl;
    }
    else {
        cout << "Open loop simulation: " << endl;
    }
    runRec(P0,visitedStates,0, numOfTransitions,localSimulation, setOfVars);
    cout << "States: " << visitedStates.size() <<
            ", Transitions: " << numOfTransitions << endl;
}
void TDS::runRec(bdd current, vector<bdd>& visitedStates, int currIndex,
                 int& numOfTransitions, bool local, bdd &setOfVars){
    if (local){
        cout << currIndex << ":"; fdd_printset(bdd_exist(current,setOfVars));
    }
    else {
        cout << currIndex << ":"; fdd_printset(current);
    }

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
                runRec(next,visitedStates,nextIndex, numOfTransitions, local, setOfVars);
            }
            else if (local){
                    cout << nextIndex << ":"; fdd_printset(bdd_exist(next,setOfVars));cout << endl;
                }
            else {
                cout << nextIndex << ":"; fdd_printset(next);cout << endl;
            }
         }
    }
}

//specification given in dijuntive form
//predicate corresponding to illegal set of states

void TDS::supcon(string filePath){
    if(!initialSet)
        setInitials();
    bdd P = spec::readSpec1File(filePath,this);
    P &= spec::readSpec2File(filePath, this);
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
void TDS::printControlData(string filePath){
    FILE* controlledBehavior;
    string directory = filePath + "/results/controlledBehavior.txt";
    controlledBehavior = fopen(directory.c_str(),"w");
    if (controlledBehavior == nullptr){
        cerr << "Error: controlledBehavior cannot be created!" << endl;
        abort();
    }
    fdd_fprintset(controlledBehavior,supC2P);
    fclose(controlledBehavior);
    for(auto& sigma: Sigma){
        if (sigma.second->isControllable()){
            FILE* sigmaEN;
            string tempDir = filePath + "/results/"+sigma.second->getName()+".txt";
            sigmaEN = fopen(tempDir.c_str(),"w");
            fdd_fprintset(sigmaEN, sigma.second->getFSigma());
            fclose(sigmaEN);
        }
    }

}
