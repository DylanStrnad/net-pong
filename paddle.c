#include "bounce.h"
#define MID_ROW 12

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <curses.h>
#include <signal.h>

//used in clients game
void paddle_init_client(struct paddle* paddlePtr){
    paddlePtr->pad_top = MID_ROW - 3;
    paddlePtr->pad_bot = MID_ROW + 2;
    paddlePtr->pad_col = LEFT_EDGE;

    //draw paddle
    for(int y = paddlePtr->pad_top; y <= paddlePtr->pad_bot; ++y){
        mvaddch(y, paddlePtr->pad_col, '#');
    }

}

//used in servers game
void paddle_init_server(struct paddle* paddlePtr){
    paddlePtr->pad_top = MID_ROW - 3;
    paddlePtr->pad_bot = MID_ROW + 2;
    paddlePtr->pad_col = RIGHT_EDGE;

    //draw paddle
    for(int y = paddlePtr->pad_top; y <= paddlePtr->pad_bot; ++y){
        mvaddch(y, paddlePtr->pad_col, '#');
    }
}

void paddle_up(struct paddle* paddlePtr, int edge){
    if(paddlePtr->pad_top != TOP_ROW){ //check if paddle is at top
        int y_cur = paddlePtr->pad_bot;
        paddlePtr->pad_top -=1;
        paddlePtr->pad_bot -=1;

        //erase bottom
        mvaddch(y_cur, edge, BLANK);

        //draw paddle
        for(int y = paddlePtr->pad_top; y <= paddlePtr->pad_bot; ++y){
            mvaddch(y, paddlePtr->pad_col, '#');
        }
    }
}
void paddle_down(struct paddle* paddlePtr, int edge){
    if(paddlePtr->pad_bot != BOT_ROW){ //check if paddle is at bottom
        int y_cur = paddlePtr->pad_top;
        paddlePtr->pad_top +=1;
        paddlePtr->pad_bot +=1;

        //erase top
        mvaddch(y_cur, edge, BLANK);

        //draw paddle
        for(int y = paddlePtr->pad_top; y <= paddlePtr->pad_bot; ++y){
            mvaddch(y, paddlePtr->pad_col, '#');
        }
    }
}

//check if ball contacts paddle. Also checks if ball intercepts paddle
int paddle_contact(int y, int x, struct paddle* paddlePtr, struct pingpongball* ballPtr, int edge){
        if(y >= paddlePtr->pad_top && y <= paddlePtr->pad_bot && x == edge){ // check if ball hits or intercepts paddle
            
            //change ball direction for client
            if(edge == LEFT_EDGE + 1){
                ballPtr->x_dir = 1;
            }
            else{
                //change ball direction for server
                ballPtr->x_dir = -1;
            }
            return CONTACT;
        } else if (x == edge){ //check if no paddle contact and at right edge
            return LOSE;
        } else{ //otherwise, there is no contact
            return NO_CONTACT;
        }
}