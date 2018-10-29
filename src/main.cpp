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

    string filePath = "Input/Drone";
    string rootFile = "DroneNetwork";
    cout << "Welcome to DES version 1.0" << endl;


    TDS temp(filePath, rootFile);
    temp.run();
    temp.supcon(filePath);
    temp.runUnderControl();

    return 0;
}
