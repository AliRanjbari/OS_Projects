#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>


char my_buffer[1024] = {0};



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
    memset(my_buffer, 0, 1024);
    sprintf(my_buffer, "Client connected!(%d)\n", client_fd);
    write(1, my_buffer, 1024);

    return client_fd;
}


int play_game(int player_fd, int waiting_player_fd, int* port){

    if(waiting_player_fd == -1){
        return player_fd;
    }
    
    char buff[10];
    memset(buff, 0, 10);
    sprintf(buff, "%d 1", ++(*port));
    send(player_fd, buff, sizeof(buff), 0);
    sprintf(buff, "%d 2", (*port));
    send(waiting_player_fd, buff, sizeof(buff), 0);

    memset(my_buffer, 0, 1024);
    sprintf(my_buffer, "player %d and %d conncted with port %d\n", player_fd, waiting_player_fd, *port);
    write(1, my_buffer, 1024);

    return -1;          // no player is waiting
}


void send_open_ports(int fd, int open_ports[10]){
    char buff[10] = {0};
    int n = 1;

    for(int i=0; i<10; i++){
        if(open_ports[i] != 0){
            memset(buff, 0, 10);
            sprintf(buff, "%d\n", open_ports[i]);
            send(fd, buff, sizeof(buff), 0);
        }
    }
    send(fd, "\0", 1, 0);

}

void add_open_port(int port, int* open_ports) {
    for(int i=0; i<10; i++)
        if(open_ports[i] == 0){
            open_ports[i] = port;
            return;
        }
}

void remove_open_port(int port, int* open_ports) {
    for(int i=0; i<10; i++)
        if(open_ports[i] == port){
            open_ports[i] = 0;
            return;
        }
}


int save_final_board(char* buff){
    int port;
    char temp[100] = {0};
    sscanf(buff, "%d\n%s", &port, temp);

    memset(my_buffer, 0, 1024);
    sprintf(my_buffer, "port %d has closed\n", port);
    write(1, my_buffer, 1024);

    int fd = open("log.txt", O_CREAT | O_WRONLY | O_APPEND);

    write(fd, buff, strlen(buff));
    write(fd, "__________________\n", 19);

    close(fd);

    return port;
}


int main(int argc, char* argv[]){

    int port;
    int open_ports[10] = {0};
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


    int server_fd, new_socket, max_sd;
    int waiting_player_fd = -1;
    fd_set master_set, working_set;

    server_fd = setup_server(port);
    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    write(1, "Server is running\n", 18);

    int port_to_close;
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
                        memset(my_buffer, 0, 1024);
                        sprintf(my_buffer, "client fd = %d closed\n", i);
                        write(1, my_buffer, 1024);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }
                    option = atoi(buff);
                    switch (option)
                    {
                    case 1:
                        waiting_player_fd = play_game(i, waiting_player_fd, &port);
                        if(waiting_player_fd == -1)
                            add_open_port(port, open_ports);
                        break;
                    case 2:
                        send_open_ports(i, open_ports);
                        break;
                    default:
                        port_to_close = save_final_board(buff);
                        remove_open_port(port_to_close, open_ports);
                        break;
                    }

                }
            }
        }
    }


    return 0;
}