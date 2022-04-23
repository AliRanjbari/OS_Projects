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
    char exec_file[] = "./class_handler.out";

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

            execl(exec_file, exec_file, str, NULL);
            perror("execl");
            exit(0);
        }

    }
    
    while(wait(0) != -1);

    wait(0);
    return 0;
}