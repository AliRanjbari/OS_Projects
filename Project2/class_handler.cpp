#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>
#include <sstream>

using namespace std;
namespace fs = std::filesystem;


string get_student_name(string path) {

    stringstream path_stream(path);
    string name;
    getline(path_stream, name, '/');
    getline(path_stream, name, '/');
    getline(path_stream, name, '.');
    return name;
}


int main(int argc, char* argv[], char* envp[]) {

    pid_t pid;
    FILE* stream;
    int parent_pipe[2];
    char* exec_argv[1];
    char str[1024];
    char exec_file[] = "./student_handler.out";
    
    if(argc < 2) {
        cerr << "Error: not path selected\n";
        return -1;
    }

    parent_pipe[0] = atoi(argv[2]);
    parent_pipe[1] = atoi(argv[3]);
    close(parent_pipe[0]);

    stream = fdopen(parent_pipe[1], "w");


    for(const auto &file : fs::directory_iterator(argv[1])) {
        
        fprintf(stream, "%s ", get_student_name(file.path()).c_str());

        pid =  fork();
        if(pid == 0) {
            execl(exec_file, exec_file, file.path().c_str(), NULL);
            exit(0);
        }
    }
    fprintf(stream, "!");

    fclose(stream);
    close(parent_pipe[1]);


    while(wait(0) != -1);
    
    return 0;
}