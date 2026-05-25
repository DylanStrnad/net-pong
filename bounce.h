#define BLANK ' '
#define DFL_SYMBOL 'o'
#define TOP_ROW 5
#define BOT_ROW 20
#define LEFT_EDGE 10
#define RIGHT_EDGE 70 
#define CONTACT 1
#define NO_CONTACT 0
#define LOSE -1

#define Y_INIT 10 /* starting col aif */
#define X_INIT 10 /* starting row Of */
#define TICKS_PER_SEC 50 /* affects speed a) */
#define X_TTM 5
#define Y_TTM 10
/** the ping pong ball **/
struct pingpongball {
    int y_pos, x_pos,
        y_tpm, x_tpm,
        y_ttg, x_ttg,
        y_dir, x_dir;
    char symbol ; 
    // bool readyToServe;
};

struct paddle{
    int pad_top, pad_bot, pad_col;
    char pad_char;
};

void paddle_init_server(struct paddle* paddlePtr);
void paddle_init_client(struct paddle* paddlePtr);
void paddle_up(struct paddle* paddlePtr, int);
void paddle_down(struct paddle* paddlePtr, int);
int paddle_contact(int y, int x, struct paddle* paddlePtr, struct pingpongball* ballPtr, int);



