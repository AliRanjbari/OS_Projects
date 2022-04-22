#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
using namespace std;




int main(int argc, char* argv[]) {


    int pipefd[2];
    if(pipe(pipefd) == -1)
        return -1;

    pid_t pid = fork();
    if(pid == -1)
        return -1;

    if(pid == 0) {
        //child 

        close(pipefd[1]);

        for(;;){
            char buff[1024];
            ssize_t rb = read(pipefd[0], buff, sizeof(buff));
            if (rb == -1)
                return -1;
            
            if(!rb)
                break;

            puts(buff);
        }
        close(pipefd[0]);


        return 0;
    }
    
    // parent
    
    close(pipefd[0]);
    for(;;) {
        char buff[1024];
        cin >> buff;
        if(!strcmp(buff, "!"))
            break;
        
        if(write(pipefd[1], buff, (strlen(buff) + 1)*
            sizeof(buff[0])) == -1) { 
                perror("write");
                return -1;
        }

    }


    close(pipefd[1]);

    if(wait(0) == -1) {
        perror("wait");
        return -1;
    }
    return 0;
}