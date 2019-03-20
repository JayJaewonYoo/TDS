#ifndef INTERFACE
#define INTERFACE

#include <unistd.h>
#include <string>

#define BUFFER_LENGTH 1024

class interface{
public:
    void init();
    int selectFile(std::string* filePathPointer, std::string* rootFilePointer);
private:
    std::string execPath;
    char tempPath[BUFFER_LENGTH];
};
#endif // INTERFACE
