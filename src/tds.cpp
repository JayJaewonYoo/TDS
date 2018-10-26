#include "tds.h"

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
            filePath + "/" + parent->getName() + ".txt" << "read!" << endl;
}

void TDS::print(){
    cout << *root;
}
