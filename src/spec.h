#ifndef SPEC_H
#define SPEC_H

#include "tds.h"

class spec{
public:
    static bdd readLiteral(istream& input, TDS* tree);
    static bdd readClause(istream& input, TDS* tree);
    static bdd readPredicate(istream& input, TDS* tree);
    static void readEventSpec(istream& input, bdd& pred, TDS* tree);
    static bdd readSpec1File(string filePath, TDS* tree);
    static bdd readSpec2File(string filePath, TDS* tree);
};

#endif // SPEC_H
