#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

void alarm_handler(int sig) {
    write(1, "tick tock\n", 10);
}

int main(int argc, char const *argv[]) {
    char buff[1024];
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);
    
    alarm(3);
    int read_ret = read(0, buff, 1024);
    alarm(0);
    printf("%d\n", read_ret);

    while (1);

    return 0;
}