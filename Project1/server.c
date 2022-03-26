#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>




/*____________IPC____________*/

int setup_server(int port){
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    listen(server_fd, 4);

    return server_fd;
}

int accept_client(int server_fd){
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);

    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);
    printf("Client connected!(%d)\n", client_fd);

    return client_fd;
}

int main(int argc, char* argv[]){

    int port;
    int server_fd, client_fd;
    char buff[1024] = {0};

    if(argc < 2){
        write(1, "Erro: you did't enter port number\n", 34);
        return 0;
    }
    else{
        port = atoi(argv[1]);
        sprintf(buff, "Connecting Server to port %d ...\n", port);
        write(1, buff, 1024);
    }

    server_fd = setup_server(port);
    int client_count = 0;

    for(;;) {
        client_fd = accept_client(server_fd);
        client_count++;

        memset(buff, 0, 1024);
        recv(client_fd, buff, 1024, 0);

        printf("Client %d said: %s\n",client_fd, buff);

        
        sprintf(buff, "Hello from server, you're client %d", client_count);
        send(client_fd, buff, strlen(buff), 0);
        // write(client_fd, "hello\n", 8);

        close(client_fd);
        
    }


    return 0;
}