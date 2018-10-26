#ifndef TDS_H
#define TDS_H

#include "des.h"

#define BDD_nodeNUM 1000000
#define BDD_cacheSize 10000

class TDS{
private:
    DESPtr root;
    unordered_map<string,ePtr> Sigma;
public:
    TDS(string fileName, string filePath){
        root.reset(inputReader(fileName, filePath));
    }

    //read input file
    DES* inputReader(string filePath, string fileName, DES* par = nullptr);
    // read components part of the input file
    void readComponents(string filePath, ifstream& is, DES* parent, int size);

    void readTransitions(ifstream& is, DES* G, int tsize){
        cout << "Reading transitions of " <<
                G->getName() << "!" << endl;
        delta* transitionStructure = new delta();
        for(int i = 0; i < tsize; ++i){
            transition* newTransition = new transition();
            int numOfSrcs, numOfDsts;
            is >> numOfSrcs >> numOfDsts;

            newTransition->addSrc(is,G,numOfSrcs);
            newTransition->setEvent(this->addEvent(is), G);
            //Sigma.insert(newTransition->setEvent(is,G));
            newTransition->addDst(is,G,numOfDsts);
            transitionStructure->addTransition(newTransition);
        }
        transitionStructure->symbolicEncoding(G, G->getAddPred());
        G->addDelta(transitionStructure);
        cout << "Transitions  of " <<
                G->getName() << "read!" << endl;
    }
    //
    event* addEvent(istream& is){
        string e;
        bool ctrb;
        is >> e >> ctrb;
        if ( Sigma[e] == nullptr)
            Sigma[e].reset(new event(e,ctrb));
        return Sigma[e].get();
    }
    //
    void test(){
        for(auto& sigma:Sigma){
            cout << sigma.first << ":"; fdd_printset(sigma.second->getNSigma()); cout << endl;
        }
    }
    //void transitionEndcoding()
    void print();
};
#endif // TDS_H
