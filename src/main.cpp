#include <iostream>
#include "des.h"
#include "event.h"
#include "transition.h"
#include "delta.h"
#include "tds.h"
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]){

    // Interface
    char execPath[1024];
    string tempPath;
    ssize_t length = readlink("/proc/self/exe", execPath, 1024);
    if(length != -1) {
	execPath[length] = '\0';
        tempPath = string(execPath);
    }
    tempPath = tempPath.substr(0, tempPath.size() - 3);

    char filename[1024];
    string zenityInstruction = string("zenity --file-selection --filename=");
    zenityInstruction += tempPath + string(" 2> /dev/null");
    FILE *f = popen(zenityInstruction.c_str(), "r");
    fgets(filename, 1024, f);
    length = remove(filename, filename+1024, '\n') - filename;
    filename[length] = 0;
    string filePath = string(filename);
    if(filePath.length() == 1 && (int)(filePath.c_str()[0]) == 1) {
	 cerr << "Error: Invalid file selection" << endl;
	 exit(EXIT_FAILURE);
    }
    filePath.erase(remove(filePath.begin(), filePath.end(), '\n'), filePath.end());
    size_t position = filePath.find(tempPath);
    if(position != string::npos) {
	    filePath.erase(position, tempPath.length());
    } else {
	 cerr << "Error" << endl;
	 exit(EXIT_FAILURE);
    }

    string rootFile = filePath.substr(filePath.find_last_of("/"));
    position = filePath.find(rootFile);
    if(position != string::npos) {
	    filePath.erase(position, rootFile.length());
    } else {
	 cerr << "Error" << endl;
	 exit(EXIT_FAILURE);
    }
    rootFile = rootFile.substr(1, rootFile.find(".", 0) - 1);
    
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
