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


const char exec_class_handler[] = "./class_handler.out";
const char exec_course_handler[] = "./course_handler.out";
const vector<string> COURSES = {"Physics", "English", "Math",
                                "Literature", "Chemistry"};


void fork_each_class(char* argv[], int pipe_fd[2]) {
    pid_t pid;
    char str[1024];
    for(const auto &file : fs::directory_iterator(argv[1])) {
        pid =  fork();

        if(pid == 0) {
            strcpy(str, file.path().c_str());

            execl(exec_class_handler, exec_class_handler, str,
                to_string(pipe_fd[0]).c_str(),
                to_string(pipe_fd[1]).c_str(), NULL);
            // perror("execl");
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
    int n_class = 3;
    do{
        c = fgetc(stream);
        if(c == '!')
            n_class--;
        else {
            temp_str[i] = c;
            i++;
        }
    }
    while(n_class);

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
    

    for(string course : COURSES) {
        pid = fork();
        if(pid == (pid_t)0) {
            execl(exec_course_handler, exec_course_handler,
                course.c_str(), names_stream, NULL);
            exit(0);
        }
    }


    while(wait(0) != -1);

    return 0;
}