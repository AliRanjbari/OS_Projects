#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>
// #include <fcntl.h>

using namespace std;
namespace fs = std::filesystem;



int main(int argc, char* argv[]) {
    
    pid_t pid;
    char* exec_argv[1];
    char str[1024];

    if(argc < 2) {
        cerr << "Error: not path selected\n";
        return -1;
    }

    string path = argv[1];
    

    for(const auto &file : fs::directory_iterator(path)) {
        cout << "found: "<< file.path() << std::endl;  
        pid =  fork();
        if(pid == 0) {
            strcpy(str, file.path().c_str());
            execve("./class_handler.out", exec_argv, NULL);
        }

    }
    
    return 0;
}