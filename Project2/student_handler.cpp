#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>
// #include <fcntl.h>

using namespace std;
namespace fs = std::filesystem;
// using std::filesystem::d irectory_iterator();




int main(int argc, char* argv[]) {

    if(argc < 2) {
        cerr << "Error: not path selected\n";
        return -1;
    }

    string path = argv[1];
    

    cout << "       " << argv[1] << endl;   
    
    return 0;
}