#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

using namespace std;
// namespace fs = std::filesystem;




int main(int argc, char* argv[]) {




    if(argc < 2) {
        cerr << "Error: not path selected\n";
        return -1;
    }

    fstream file;
    string temp;
    FILE* fifo_write;
    
    file.open(argv[1], ios::in);
    for(int i=0; i<5;i++) {
        string course, grade;
        string fifo_name;
        getline(file, course, ',');
        getline(file, grade, '\n');
        // cout << course  << endl;
        // cout << grade << endl;
        fifo_name = course + "_" + argv[1];
        fifo_write = fopen(fifo_name.c_str(), "w");
        // perror("fopen");
        
        fwrite(grade.c_str(), 1,
            strlen(grade.c_str()), fifo_write);
        fclose(fifo_write);
    }
    file.close();
    

    return 0;
}