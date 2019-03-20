#include <iostream>
#include "des.h"
#include "event.h"
#include "transition.h"
#include "delta.h"
#include "tds.h"
#include "interface.h"

using namespace std;

int main(int argc, char *argv[]){

    // Interface
    interface gui;
    gui.init();
    string filePath;
    string rootFile;
    int exit_code = gui.selectFile(&filePath, &rootFile);
    if(exit_code == -1) {
	    cerr << "Error: Invalid file selection. Make sure the selected file not within directory of TDS or not within one of its subdirectories" << endl;
	    exit(EXIT_FAILURE);
    }
    
    bdd_init(BDD_nodeNUM,BDD_cacheSize);

    bdd_setvarnum(10000);
    fdd_file_hook(printhandler);

    /*
    if (argc != 3){
        cerr << "Error: invalid command line arguments!" << endl;
        abort;
    }
    
    string filePath = argv[1];
    string rootFile = argv[2];
    
    string filePath;
    string rootFile;
    cout << "Enter directory path containing input files ie: Input/Example" << endl;
    cin >> filePath;
    cout << "Enter the name of the root file ie: root" << endl;
    cin >> rootFile;
    */
    /*
    string filePath = argv[1];
    string rootFile = argv[2];
    cout << filePath << endl;
    cout << rootFile << endl;
    */

    cout << "Welcome to DES version 1.1" << endl;

    TDS temp(filePath, rootFile);
    temp.print();
    temp.run();

    temp.supcon(filePath);
    temp.runUnderControl();
    temp.printControlData(filePath);

    temp.printADSsupervisor(filePath, rootFile);
    // temp.printADSinputs(filePath, rootFile);

    return 0;
}
