#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>

#define ROWS 6
#define COLS 7

void clear_screen(void);

void print_board(char board[ROWS][COLS]);

int drop_checker(char board[ROWS][COLS], int col, char token);

int check_direction(char board[ROWS][COLS], int r, int c, int dr, int dc, char token);

int is_winner(char board[ROWS][COLS], char token);

int is_draw(char board[ROWS][COLS]);

int read_column_choice(char player);

int bot_choose_random_col(char board[ROWS][COLS]);

int bot_choose_medium_col(char board[ROWS][COLS], char bot_token, char human_token);

int bot_choose_impossible_col(char board[ROWS][COLS], char bot_token, char human_token);
#endif
