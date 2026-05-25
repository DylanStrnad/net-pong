/* CS4-53203: Systems Programming \
/* Name:Dylan Strnad\
/* Date:\
/* Assignment# StrnadDylan-CS43203-pong-Final.txt\
\
\
\
/* Template for assignments. */\
\
\
\
/* Add your work for this assignment here\
}*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <curses.h>
#include <signal.h>
#include "bounce.h"
#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>
#include <unistd.h>


#define WAIT 0
#define PLAY 1

struct pingpongball the_ball ;
struct paddle the_paddle;


void serve();
void set_up();
void wrap_up(); 
int set_ticker(int);
void draw_walls();
void draw_game_info();
void change_speed(int);

int bounce_or_lost(struct pingpongball*, int serverSocket);

const int MAXNUM = 6;
int balls_left = 3;
int score = 0;
char message[32];

struct sockaddr_in serverAddress;
int serverSocket;
int clientSocket;

int state;

int main(int argc, char *argv[]){
    printf("waiting...\n");

    int port = atoi(argv[1]);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    //makes the server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    //fixes issues with socket not being available
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));  

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 1);

    clientSocket = accept(serverSocket, NULL, NULL);

    //exchange introduction messages
    char name[15];
    printf("Enter name.");
    scanf("%s", name);

    char msg[128];
    int net_length = 15;
    sprintf(msg, "HELO 1.0 %d %d %s", TICKS_PER_SEC, net_length, name);
    send(clientSocket, msg, sizeof(msg), 0);

    char data[128];
    recv(clientSocket, data, sizeof(data), 0);
    printf("Message: %s\n", data);

    sprintf(msg, "SERV %d", balls_left);
    send(clientSocket, msg, sizeof(msg), 0);
    sleep(3);
    //end of introduction

    srand(getpid());
    int c;
    int state = WAIT;
    set_up(); /* init all stuff */
    serve(); /* start up ball */

    while ((c = getchar()) != 'q') {
        if (c == 'j'){
            paddle_down(&the_paddle, RIGHT_EDGE);
        }
        else if (c == 'k'){
            paddle_up(&the_paddle, RIGHT_EDGE);
        }
    }
    wrap_up();
}

void set_up()
{
    void ball_move (int) ;
    the_ball.y_pos = Y_INIT;
    the_ball.x_pos = X_INIT;
    the_ball.y_ttg = the_ball.y_tpm = Y_TTM ;
    the_ball.x_ttg = the_ball.x_tpm = X_TTM;
    the_ball.y_dir = 1;
    the_ball.x_dir = 1;
    the_ball.symbol = DFL_SYMBOL;
    initscr();
    crmode ();

    signal( SIGINT , SIG_IGN );
    paddle_init_server(&the_paddle);
    draw_walls();
    refresh ();
    signal( SIGALRM, ball_move );
    set_ticker( 1000 / TICKS_PER_SEC ); /* send millisecs per tick */
}

void wrap_up()
{
    set_ticker( 0 );
    clear();
    refresh();
    endwin(); /* put back to normal */
}

void ball_move(int signum)
{
    draw_game_info();
    int y_cur, x_cur, moved;
    signal( SIGALRM , SIG_IGN ); /* dont get caught now */

    if(state == WAIT){
        ssize_t readBallData = recv(clientSocket, &the_ball, sizeof(struct pingpongball), 0);
        if(readBallData == sizeof(struct pingpongball)){
            state = PLAY;
            the_ball.x_pos = LEFT_EDGE + 1;
        } else if (readBallData > 0){ //recieved the miss message
            strcpy(message, "opponent missed, time to serve");
            the_ball.y_pos = Y_INIT + 5;
            the_ball.x_pos = RIGHT_EDGE - 5;
            serve();
            the_ball.y_dir = -1;
            the_ball.x_dir = -1;
            mvaddch(the_ball.y_pos, the_ball.x_pos, the_ball.symbol);
            
            state = PLAY;
            --balls_left;
            ++score;
            draw_game_info();
            refresh ();
            //send DONE messgae
            if(balls_left == 0){
                    wrap_up();
                    printf("DONE!\n");
                    exit(1);
            }
        } else{
            printf("other player disconnected\n");
            wrap_up();
            exit(0);
        }
    }
    if (state == PLAY){
        //remove old message
        strcpy(message, "");

        y_cur = the_ball.y_pos;
        x_cur = the_ball.x_pos;
        moved = 0 ;
        if(the_ball.y_tpm > 0 && the_ball.y_ttg-- == 1){
            the_ball.y_pos += the_ball.y_dir ; /* move */
            the_ball.y_ttg = the_ball.y_tpm;
            moved = 1; 
        }
        if ( the_ball.x_tpm > 0 && the_ball.x_ttg-- == 1){
            the_ball.x_pos += the_ball.x_dir ; /* move */
            the_ball.x_ttg = the_ball.x_tpm;
            moved = 1;
        } 
        if(moved){
            //when the ball hits the net, replace with blank
            if (the_ball.x_pos == LEFT_EDGE){
                mvaddch(y_cur, x_cur, BLANK);
                mvaddch(the_ball.y_pos, the_ball.x_pos, BLANK);
            } else{
                mvaddch(y_cur, x_cur, BLANK);
                mvaddch(y_cur, x_cur, BLANK);
                mvaddch(the_ball.y_pos, the_ball.x_pos, the_ball.symbol);
            }
            
            //ball goes out of bounds, remove ball, reset position
            if(bounce_or_lost(&the_ball, clientSocket) == LOSE){ 
                // send the MISS message to the other player
                char msg[] = "missed";
                send(clientSocket, msg, strlen(msg), 0);

                mvaddch(the_ball.y_pos, the_ball.x_pos, BLANK);
                --balls_left;

                state = WAIT;
                //reponse to DONE
                if(balls_left == 0){
                    wrap_up();
                    printf("DONE! press q to quit\n");
                    char ch;
                    while (1) {
                        scanf(" %c", &ch);
                        if (ch == 'q') {
                            exit(0);
                        }
                    }
                }

                draw_game_info();
                refresh ();
            }

            move(LINES-1, COLS-1);
            refresh();
        }
    }
    signal(SIGALRM, ball_move);
}

int bounce_or_lost(struct pingpongball *bp, int serverSocket){
    int return_val = NO_CONTACT;

    if (bp->y_pos == TOP_ROW) {
        bp->y_dir = 1;
        return_val = CONTACT;
    } else if (bp->y_pos == BOT_ROW) {
        bp->y_dir = -1;
        return_val = CONTACT;
    }

    if (bp->x_pos == LEFT_EDGE) {
        //pass ball through socket to the other process.
        send(serverSocket, bp, sizeof(*bp), 0);
        state = WAIT;
        draw_game_info();
        refresh();
        
    } else { //check of ball is in front of paddle
        return_val = paddle_contact(bp->y_pos, bp->x_pos, &the_paddle, bp, RIGHT_EDGE - 1);
        //change speed when ball hits paddle
        change_speed(return_val);
        
    }
    refresh();
    return return_val;
}

//changes speed of ball
void change_speed(int value){
    if (value == CONTACT){ // if paddle contact, change speed of ball
        int rateChangeX = (rand() % 3);
        int rateChangeY = (rand() % 2);

        if((the_ball.x_tpm - rateChangeX) > 0 && (the_ball.y_tpm - rateChangeY) > 0){ //make sure not 0, so ball moves
            //substracting makes faster
            the_ball.x_tpm -= rateChangeX;
            the_ball.y_tpm -= rateChangeY;
        }
    }
}

int set_ticker(int n_msecs){
    struct itimerval new_timeset;
    long n_sec, n_usecs;

    n_sec = n_msecs / 1000;              /* int part */
    n_usecs = (n_msecs % 1000) * 1000L;  /* remainder */

    new_timeset.it_interval.tv_sec  = n_sec;   /* set reload */
    new_timeset.it_interval.tv_usec = n_usecs; /* new ticker value */
    new_timeset.it_value.tv_sec     = n_sec;   /* store this */
    new_timeset.it_value.tv_usec    = n_usecs; /* and this */

    return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

void draw_walls(){
    for(int x = LEFT_EDGE; x < RIGHT_EDGE; ++x){
        mvaddch(TOP_ROW -1, x, '-');
    }
    for(int y = TOP_ROW; y < BOT_ROW; ++y){
        mvaddch(y, LEFT_EDGE - 1, '|');
    }
    for(int x = LEFT_EDGE; x < RIGHT_EDGE; ++x){
        mvaddch(BOT_ROW + 1, x, '-');
    }
}

void draw_game_info(){
    mvprintw(BOT_ROW + 3, LEFT_EDGE, "Balls Left: %d | ", balls_left);
    if (state == 1){
        mvprintw(BOT_ROW + 3, LEFT_EDGE + 16, "State: playing | ");
    } else {
        mvprintw(BOT_ROW + 3, LEFT_EDGE + 16, "State: waiting | ");
    }
    mvprintw(BOT_ROW + 3, LEFT_EDGE + 35, "Message: %s", message);
    mvprintw(BOT_ROW + 5, LEFT_EDGE, "Score: %d", score);
}

void serve(){
    //higher changeRate means slower, lower means faster
    int changeRateX = rand()%MAXNUM + 2;
    int changeRateY = rand()%MAXNUM + changeRateX;
    the_ball.y_ttg = the_ball.y_tpm = changeRateY;
    the_ball.x_ttg = the_ball.x_tpm = changeRateX;
}