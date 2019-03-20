#include "interface.h"
#include <iostream>
#include <unistd.h>
#include <algorithm>

using namespace std;

void interface::init(){
    ssize_t length = readlink("/proc/self/exe", tempPath, BUFFER_LENGTH);
    if(length != -1) {
	tempPath[length] = '\0';
        execPath = string(tempPath);
    }
    execPath = execPath.substr(0, execPath.size() - 3);
}

int interface::selectFile(string* filePathPointer, string* rootFilePointer) {

    char filename[BUFFER_LENGTH];
    string zenityInstruction = string("zenity --file-selection --filename=");
    zenityInstruction += tempPath + string(" 2> /dev/null");
    FILE *f = popen(zenityInstruction.c_str(), "r");
    fgets(filename, BUFFER_LENGTH, f);
    ssize_t length = remove(filename, filename+BUFFER_LENGTH, '\n') - filename;
    filename[length] = 0;
    string filePath = string(filename);
    cout << filePath.length() << endl;
    cout << (int)(filePath.c_str()[0]) << endl;
    if(filePath.length() == 1 && (int)(filePath.c_str()[0]) == 1) {
	 return -1; // Failure due to invalid selection, most likely caused by simply closing the file select dialog
    }
    filePath.erase(remove(filePath.begin(), filePath.end(), '\n'), filePath.end());
    size_t position = filePath.find(execPath);
    if(position != string::npos) {
	    filePath.erase(position, execPath.length());
    } else {
	 return -1; // Failure due to selected file not being within executable directory or one of its subdirectories
    }

    string rootFile = filePath.substr(filePath.find_last_of("/"));
    position = filePath.find(rootFile);
    if(position != string::npos) {
	    filePath.erase(position, rootFile.length());
    } else {
	 return -1; // Failure due to unknown reasons
    }
    rootFile = rootFile.substr(1, rootFile.find(".", 0) - 1);
    *filePathPointer = filePath;
    *rootFilePointer = rootFile;
    return 0; // Success
}
