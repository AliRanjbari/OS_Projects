#include <iostream>
#include <vector>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
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
        string temp = course_name + "_" + name;
        fifo_names.push_back(temp);
        mkfifo(temp.c_str(), S_IRWXU);
        // perror("mkfifo");
    }
    
    

    FILE* fifo_read;
    char str[32];
    int sum_grade = 0;
    for(string fn : fifo_names) {
        cout << fn << endl;
        fifo_read = fopen(fn.c_str(), "r");
        
        perror("fopen");
        cout << fn << endl;
        // fread(str, sizeof(char), strlen(str), fifo_read);
        // perror("fread");
        // sum_grade += atoi(str);
        fclose(fifo_read);
        remove(fn.c_str());
        // perror("remove");
    }
    


    return 0;
}