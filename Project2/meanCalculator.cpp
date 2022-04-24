#include <iostream>
#include <vector>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>
// #include <fcntl.h>


using namespace std;
namespace fs = std::filesystem;


const char exec_file[] = "./class_handler.out";


void fork_each_class(char* argv[], int pipe_fd[2]) {
    pid_t pid;
    char str[1024];
    for(const auto &file : fs::directory_iterator(argv[1])) {
        pid =  fork();
        if(pid == 0) {
            strcpy(str, file.path().c_str());

            execl(exec_file, exec_file, str,
                to_string(pipe_fd[0]).c_str(),
                to_string(pipe_fd[1]).c_str(), NULL);
            perror("execl");
            exit(0);
        }
    }
}


char* read_pipe(int pipe_fd[2]) {
    FILE* stream;
    char* temp_str = (char*)malloc(sizeof(char) * 1024);
    stream = fdopen(pipe_fd[0], "r");
    int c;
    int i = 0;
    while((c = fgetc(stream)) != EOF){
        temp_str[i] = c;
        i++;
    }

    fclose(stream);
    return temp_str;
}


int main(int argc, char* argv[]) {
    
    pid_t pid;
    int pipe_fd[2];
    char* exec_argv[1];
    char str[1024];

    if(argc < 2) {
        cerr << "Error: not path selected\n";
        return -1;
    }

    pipe(pipe_fd);

    fork_each_class(argv, pipe_fd);
    
    close(pipe_fd[1]);
    

    // get student names
    char* names_stream = read_pipe(pipe_fd);
    close(pipe_fd[0]);
    
    char* token_value = strtok(names_stream, " ");
    vector<string> names;
    while(token_value != NULL) {
        names.push_back(token_value);
        token_value = strtok(NULL, " ");
    }

    for(auto i: names)
        cout << i << endl;

    while(wait(0) != -1);

    return 0;
}