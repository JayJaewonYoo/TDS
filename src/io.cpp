#include "io.h"
#include "tds.h"


DES* IO::inputReader(string filePath, string fileName, DES* par, TDS* G){
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
        IO::readComponents(filePath,is,newDES,size,G);
        IO::readTransitions(is,newDES,tsize,G);
    }
    else if ( type == "sync" ){
        newDES = new syncDES(name,par,size,fddState);
        IO::readComponents(filePath,is,newDES,size,G);
    }
    else {
        cerr << "Error: Unkown type " << type << " in " <<
                filePath + "/" + name + ".txt" << endl;
        abort();
    }
    cout << "Inupt file" << filePath + "/" + name + ".txt" << " read!" << endl;
    return newDES;
}


void IO::readComponents(string filePath, ifstream& is, DES* parent, int size, TDS* G){
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
           newDES = IO::inputReader(filePath, name,parent,G);
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

void  IO::readTransitions(ifstream& is, DES* G, int tsize,TDS* tree){
    cout << "Reading transitions of " <<
            G->getName() << "!" << endl;
    delta* transitionStructure = new delta();
    for(int i = 0; i < tsize; ++i){
        transition* newTransition = new transition();
        int numOfSrcs, numOfDsts;
        is >> numOfSrcs >> numOfDsts;

        newTransition->addSrc(is,G,numOfSrcs);
        newTransition->setEvent(tree->addEvent(is), G);
        newTransition->addDst(is,G,numOfDsts);
        transitionStructure->addTransition(newTransition);
    }
    transitionStructure->symbolicEncoding(G, G->getAddPred());
    G->addDelta(transitionStructure);
    cout << "Transitions  of " <<
            G->getName() << " read!" << endl;
}
