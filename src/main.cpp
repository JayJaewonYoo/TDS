#include <iostream>
#include "des.h"
#include "event.h"
#include "transition.h"
#include "delta.h"
#include "tds.h"

using namespace std;


int main(int argc, char *argv[]){

    bdd_init(BDD_nodeNUM,BDD_cacheSize);
    bdd_setvarnum(10000);
    fdd_file_hook(printhandler);

    cout << "Welcome to DES version 1.0" << endl;

    TDS temp("Examples/Input/Test1", "X0");

    temp.print();
    temp.test();

    return 0;
}
