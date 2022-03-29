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
    int server_fd, new_socket, max_sd;
    char buff[1024] = {0};
    fd_set master_set, working_set;

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
    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    write(1, "Server is running\n", 18);

    for(;;) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for(int i=0; i <= max_sd; i++){
            if(FD_ISSET(i, &working_set)) {

                if(i == server_fd) {
                    new_socket = accept_client(server_fd);
                    FD_SET(new_socket, &master_set);
                    if(new_socket > max_sd)
                        max_sd = new_socket;
                    memset(buff, 0, 1024);
                    sprintf(buff, "New client connected(%d)\n" ,new_socket);
                    write(1, buff, 1024);   
                }

                else{
                    int bytes_recieved;
                    memset(buff, 0, 1024);
                    bytes_recieved = recv(i, buff, 1024, 0);
                    int option = 10;
                    if(bytes_recieved == 0) {
                        printf("client fd = %d closed\n", i);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }
                    else {
                        option = atoi(buff);
                        printf("client %d: %d\n", i, option);
                        // memset(buff, 0, 1024);
                    }

                }
            }
        }
    }


    return 0;
}