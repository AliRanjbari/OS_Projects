#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;


//https://www.ibm.com/docs/en/zos/2.3.0?topic=io-using-named-pipes

int main(int argc, char* argv[]) {

    int ret_value, c_status;
    char fifoname[] = "temp.fifo";
    size_t n_elements;
    char char_ptr[32];
    char str[] = "string for fifo";
    FILE *rd_stream, *wr_stream;


    mkfifo(fifoname, S_IRWXU);

    pid_t pid = fork();
    if(pid == 0) {
        wr_stream = fopen(fifoname, "w");

        n_elements = fwrite(str, 1, strlen(str), wr_stream);
        exit(0);

    }
    else {
        rd_stream = fopen(fifoname, "r");
        ret_value = fread(char_ptr, sizeof(char), strlen(str), rd_stream);
        ret_value = fclose(rd_stream);
        ret_value = remove(fifoname);
        cout << char_ptr;

    }

    pid = wait(0);

    exit(0);
    
    return 0;
}