#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
// #include <filesystem>

using namespace std;
// namespace fs = std::filesystem;




int main(int argc, char* argv[]) {


    // int parent_pipe[2];
    fstream file;

    if(argc < 2) {
        cerr << "Error: not path selected\n";
        return -1;
    }

    // parent_pipe[0] = atoi(argv[2]);
    // parent_pipe[1] = atoi(argv[3]);
    // close(parent_pipe[0]);    

    // cout << "       " << argv[1] << endl;
    

    file.open(argv[1], ios::in);
    if(file.is_open()) {
        string tp;
        while(getline(file, tp, ',')) {
            // cout << tp << "\n";
        }
        file.close();
    }
    

    return 0;
}