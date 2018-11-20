#ifndef IO_H
#define IO_H

#include <iostream>

using namespace std;
class DES;
class TDS;


class IO{
public:
    static DES* inputReader(string filePath, string fileName, DES* par, TDS* G);
    static void readComponents(string filePath, ifstream& is, DES* parent, int size, TDS* G);
    static void  readTransitions(ifstream& is, DES* G, int tsize,TDS* tree);
};

#endif // IO_H
