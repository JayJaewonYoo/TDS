#include "spec.h"

bdd spec::readLiteral(istream& input, TDS* tree){
    string var, value;
    input >> var >> value;
    DES* D1 = tree->findComponent(var);
    DES* D2 = tree->findComponent(value);
    if (D1 == nullptr || D2 == nullptr){
        string error = (D1)? value:var;
        cerr << "Error: DES " << error << " not found in spec file!" << endl;
        abort();
    }
    if (D1->getType() != DEStype::XOR){
        cerr << "Error: DES " << var << " is not an XOR product DES!" << endl;
        abort();
    }
    return (fdd_ithvar(D1->getfddDomain(), D2->getfddState()) &
            D1->getAddPred());
}

//read a clause of a DNF
bdd spec::readClause(istream& input, TDS* tree){
    int sizeOfClause;
    input >> sizeOfClause;
    bdd clause = bddtrue;
    while(sizeOfClause){
        bdd  literal = spec::readLiteral(input, tree);
        clause &= literal;
        sizeOfClause--;
    }
    return clause;
}

// Read a DNF (disjuntive normal form) predicate
bdd spec::readPredicate(istream& input, TDS* tree){
    int numOfClauses;
    input >> numOfClauses;
    bdd predicate = bddfalse;
    while(numOfClauses){
        bdd clause = spec::readClause(input, tree);

        predicate |= clause;
        numOfClauses--;
    }
    return predicate;
}

bdd spec::readSpec1File(string filePath, TDS* tree){
    ifstream input(filePath + "/spec1.txt");
    if (input.fail()){
        cerr << "Error: spec1 file not found!" << endl;
        abort();
    }
    bdd predicate = spec::readPredicate(input, tree);
    return bdd_not(predicate);
}

void spec::readEventSpec(istream& input, bdd& pred, TDS* tree){
    string s;
    input >> s;
    event* sigma = tree->findEvent(s);
    if (sigma == nullptr){
        cerr << "Error: Event " << s << " in spec2 not found!" << endl;
        abort();
    }
    bdd predicate = spec::readPredicate(input, tree);
    pred |= sigma->handleIllegalPredicate(predicate);
}


bdd spec::readSpec2File(string filePath, TDS* tree){
    ifstream input(filePath + "/spec2.txt");
    if (input.fail()){
        cerr << "Error: spec1 file not found!" << endl;
        abort();
    }
    int numOfSpecs;
    input >> numOfSpecs;
    bdd predicate = bddfalse;
    while (numOfSpecs){
        spec::readEventSpec(input,predicate, tree);
        numOfSpecs--;
//        fdd_printset(predicate); cout << endl;
    }
    return bdd_not(predicate);
}
