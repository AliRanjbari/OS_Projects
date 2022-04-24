#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
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

    string student_name = get_student_name(argv[1]);

    fstream file;
    string temp;
    FILE* fifo_write;
    
    // sleep(1);
    file.open(argv[1], ios::in);
    for(int i=0; i<5;i++) {
        string course, grade;
        string fifo_name;
        getline(file, course, ',');
        getline(file, grade, '\n');
        // cout << course  << endl;
        // cout << grade << endl;
        
        fifo_name = course + "_" + student_name;
        // cout << fifo_name << endl;
        // fifo_write = fopen(fifo_name.c_str(), "w");
        
        // fwrite(grade.c_str(), 1,
        //     strlen(grade.c_str()), fifo_write);
        // fclose(fifo_write);
    }
    file.close();
    

    return 0;
}