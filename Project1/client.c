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

void change_turn(Game* g){
    if(g->turn == 1)        // change the turn
        g->turn = 2;
    else
        g->turn = 1;

    g->move_left--;
}

void get_move(int *x, int *y){
    char buff[1024] = {0};
    write(1, "Your move (x y): ", 17);
    for(;;){
        memset(buff, 0, 1024);
        read(0, buff, 1024);
        sscanf(buff, "%d %d", x, y);
        // printf("%d ")
        if(*x < 1 || *x > 3 || *y < 1 || *y > 3)
            printf("invalid move try again\n");
        else
            break;
    }
}

void move(Game* g){
    int x, y;

    for(;;){
        get_move(&x, &y);
        x--;            // move has gotten between 1 and 3
        y--;
        if(g->board[x][y]){
            printf("you can't mark here\n");          //failed
            continue;
        }
        
        g->board[x][y] = g->turn;
        change_turn(g);
        break;
    }
}


char* board_to_string(Game* g){
    char* buff = (char*)malloc(sizeof(char)*20);
    sprintf(buff, "%d %d %d\n%d %d %d\n%d %d %d\n",  g->board[0][0],
                                                     g->board[0][1],
                                                     g->board[0][2],
                                                     g->board[1][0],
                                                     g->board[1][1],
                                                     g->board[1][2],
                                                     g->board[2][0],
                                                     g->board[2][1],
                                                     g->board[2][2]);


    return buff;
}

void string_to_bord(Game* g, char* buff){
    sscanf(buff, "%d %d %d\n%d %d %d\n %d %d %d\n", &g->board[0][0],
                                                    &g->board[0][1],
                                                    &g->board[0][2],
                                                    &g->board[1][0],
                                                    &g->board[1][1],
                                                    &g->board[1][2],
                                                    &g->board[2][0],
                                                    &g->board[2][1],
                                                    &g->board[2][2]);
}

Game* start_game(){

    Game* game = (Game*)malloc(sizeof(Game));

    for(int i=0; i<3; i++)          // initilize board with zero
        for(int j=0; j<3; j++)
            game->board[i][j] = 0;

    game->turn = 1;
    game->move_left = 9;


    return game;
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


void play(int port, int player_number, int server_fd){

    int sock, broadcast = 1, opt = 1;
    char buffer[1024] = {0};
    struct sockaddr_in bc_address;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("192.168.1.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));

    printf("------------------------------------------------\n");
    printf("Your now playing with another player in port(%d %d)\n", port, sock);
    if(player_number == 1)
        printf("your turn\n");


    Game* g = start_game();
    int just_send_message = 0;
    int winner;
    for(;;){           //main for, for playing game
        winner = has_game_finished(g);
        if(winner != -1)
            break;         

        if(g->turn == player_number && just_send_message == 0){
            printf("your turn to move\n");
            move(g);
            char* board_str = board_to_string(g);
            sendto(sock, board_str, 20, 0,
                        (struct sockaddr*)&bc_address, sizeof(bc_address));
            free(board_str);
            just_send_message = 1;
        }
        else {
            printf("waiting for opponent to move\n");
            recv(sock, buffer, 1024, 0);
            if(just_send_message == 0) {                  // check to not recieve our own message
                string_to_bord(g, buffer);
                change_turn(g);
            }
            just_send_message = 0;
        }

        if(just_send_message == 0) {
            char* board_str = board_to_string(g);
            printf("%s", board_to_string(g));
            free(board_str);
        }
    }

    memset(buffer, 0, 1024);
    sprintf(buffer, "Winner is player %d\n", winner);
    if(player_number == 1){
        sendto(sock, buffer, 20, 0,
                            (struct sockaddr*)&bc_address, sizeof(bc_address));
        char* board_str = board_to_string(g);

        sprintf(buffer, "%d\n%s", port, board_str);
        send(server_fd, buffer, sizeof(buffer), 0);
        free(board_str);
    }
    write(1, buffer, 1024);
    free(g);
    close(sock);
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
        write(1, "\nopt: ", 6);
        memset(buff, 0, 1024);
        read(0, buff, 1024);
        int option = atoi(buff);

        switch (option)
        {
        case 1:
            send(fd, "1", 1, 0);
            printf("Waiting for another player to join ...\n");
            recv(fd, buff, 1024, 0);
            int game_port, player_number;
            sscanf(buff, "%d %d", &game_port, &player_number);
            play(game_port, player_number, fd);
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