#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

/*____________GAME____________*/

int DIR[8][3][2] = {{{0,0},{0,1},{0,2}},
                    {{1,0},{1,1},{1,2}},
                    {{2,0},{2,1},{2,2}},
                    {{0,0},{1,0},{2,0}},
                    {{0,1},{1,1},{2,1}},
                    {{0,2},{1,2},{2,2}},
                    {{0,0},{1,1},{2,2}},
                    {{0,2},{1,1},{2,0}}};

typedef struct TickTockToe {
    int board[3][3];
    int turn;
    int move_left;

} Game;

int has_game_finished(Game* g){ 
    
    for(int i =0; i < 8; i++){
        int x1 = DIR[i][0][0], y1 = DIR[i][0][1];
        int x2 = DIR[i][1][0], y2 = DIR[i][1][1];
        int x3 = DIR[i][2][0], y3 = DIR[i][2][1];
        if(g->board[x1][y1] == g->board[x2][y2] && g->board[x2][y2] == g->board[x3][y3])
            if(g->board[x1][y1])
                return g->board[x1][y1];   
    }
    if(g->move_left == 0)
        return 0;           // draw       

    return -1;          // not finitshed
    
}

int move(Game* g, int x, int y){
    if(g->board[x][y])
        return 0;           //failed
    
    g->board[x][y] = g->turn;

    if(g->turn == 1)        // change the turn
        g->turn = 2;
    else
        g->turn = 1;

    return 1;
}



/*____________IPC____________*/


int connect_server(int port){
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0){
        printf("Error in connecting to server\n");
        exit(0);
    }
    else
        printf("connected to server\n");

    return fd;
}




int main(int argc, char* argv[]){
    int port;
    int fd;
    char buff[1024] = {0};

    if(argc < 2){
        write(1, "Erro: you did't enter port number\n", 34);
        return 0;
    }
    else{
        port = atoi(argv[1]);
        sprintf(buff, "Connecting to Server(%d) ... \n", port);
        write(1, buff, 1024);
    }

    fd = connect_server(port);
    printf("Select one option: \n1. Playing\n2. Watching\n3. exit\n");

    for(;;){
        // printf("opt: ");
        write(1, "\nopt: ", 6);
        memset(buff, 0, 1024);
        read(0, buff, 1024);
        int option = atoi(buff);

        switch (option)
        {
        case 1:
            printf("Waiting for another player to join ...\n");
            send(fd, "1", 1, 0);
            break;
        case 2:
            printf("Getting all open ports ...\n");
            send(fd, "2", 1, 0);
            break;
        case 3:
            write(1, "Goodby\n", 7);
            return 0;
            break;
        default:
            printf("Not a valid option\n");
            break;
        }




        // read(0, buff, 1024);
        // send(fd, buff, strlen(buff), 0);
        // memset(buff, 0, 1024);
        // recv(fd, buff, 1024, 0);
        // write(1, buff, sizeof(buff));
        // memset(buff, 0, 1024);
    }
    return 0;
}