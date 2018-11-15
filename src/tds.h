#ifndef TDS_H
#define TDS_H

#include "des.h"

#define BDD_nodeNUM 1000000
#define BDD_cacheSize 10000

class TDS{
public:
    TDS(string fileName, string filePath);
    //read input file
    DES* inputReader(string filePath, string fileName, DES* par = nullptr);
    // read components part of the input file
    void readComponents(string filePath, ifstream& is, DES* parent, int size);
    // read transitions part of the input file
    void readTransitions(ifstream& is, DES* G, int tsize);
    //
    event* addEvent(istream& is);

    void setInitials();
    // open-loop run of the system
    void run(string G = "");
    void runRec(bdd current, vector<bdd>& visitedStates,
                int currIndex, int& numOfTransitions,
                bool local, bdd& setOfVars);

    //specification given in dijuntive form
    //predicate corresponding to illegal set of states
    bdd readSpecFile(string filePath);

    // sysnthesis functions
    void supcon(string filePath);
    void createSupC2P(bdd& P);
    bdd CR(bdd& P);
    bdd supCP(bdd& P);
    bdd Bracket(bdd& P);
    // closed-loop run of the system
    void runUnderControl();
    void runUnderControlRec(bdd current, vector<bdd>&
                            visitedStates, int currIndex, int& numOfTransitions);
    //
    void print();
    void printControlData(string filePath);
private:
    DESPtr root;
    unordered_map<string,ePtr> Sigma;
    bool initialSet;// initial symbolic procedures done if "true"!
    bdd P0;//Initial predicate
    bdd Pm;//Marker predicate
    bdd supC2P;//Supremal controllable behavior
};
#endif // TDS_H
