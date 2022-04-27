#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>

using namespace std;

string get_student_name(string path) {

    stringstream path_stream(path);
    string name;
    getline(path_stream, name, '/');
    getline(path_stream, name, '/');
    getline(path_stream, name, '.');
    return name;
}


int main(int argc, char* argv[]) {


    if(argc < 2) {
        cerr << "Error: not path selected\n";
        return -1;
    }

    fstream file;
    int fd;
    

    file.open(argv[1], ios::in);
    for(int i=0; i<5;i++) {
        string course, grade;
        string fifo_name;
        getline(file, course, ',');
        getline(file, grade, '\n');
        fifo_name = course + "_" + get_student_name(argv[1]);
        mkfifo(fifo_name.c_str(), 0666);
        fd = open(fifo_name.c_str(), O_WRONLY);
        
        write(fd, grade.c_str(),
            strlen(grade.c_str()) + 1);
        close(fd);
    }
    file.close();
    

    return 0;
}