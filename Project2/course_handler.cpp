#include <iostream>
#include <algorithm>
#include <vector>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <filesystem>


using namespace std;



int main(int argc, char* argv[]) {
    

    string course_name = argv[1];
    char* token_value = strtok(argv[2], " ");
    vector<string> names;
    while(token_value != NULL) {
        names.push_back(token_value);
        token_value = strtok(NULL, " ");
    }

    vector<string> fifo_names;

    // named pipe
    for(auto name : names) {
        string temp = course_name +"_" + name;
        fifo_names.push_back(temp);
    }
    
    char str[32];
    int fd;
    int sum_grade = 0;
    fifo_names.erase(fifo_names.begin() + fifo_names.size()-1);
    // sort(fifo_names.begin(), fifo_names.end());
    // for(auto i : fifo_names)
    //     cout << i << endl;


    for(string fn : fifo_names) {
        do
            fd = open(fn.c_str(), O_RDONLY);
        while(fd == -1);

        read(fd, str, 32);
        sum_grade += atoi(str);
        close(fd);
        remove(fn.c_str());
    }
    
    cout << "average of " << course_name << "is :";
    cout << sum_grade / fifo_names.size() << endl;
    return 0;
}

