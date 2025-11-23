#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "helper.h"

static int read_mode_choice(void) {
    char line[64];
    for (;;) {
        printf("Select mode: 1) Human vs Human  2) Human vs Bot : ");
        if (!fgets(line, sizeof(line), stdin)) return 1;
        size_t len = strlen(line);
        if (len && line[len - 1] == '\n') line[len - 1] = '\0';
        errno = 0;
        char *endp = NULL;
        long v = strtol(line, &endp, 10);
        if (endp == line || errno == ERANGE || *endp != '\0' || (v != 1 && v != 2)) { puts("Please enter 1 or 2."); continue; }
        return (int)v;
    }
}

static int read_difficulty_choice(void) {
    char line[64];
    for (;;) {
        printf("Choose bot difficulty: 1) Low (random)  2) Medium  3) Impossible : ");
        if (!fgets(line, sizeof(line), stdin)) return 1;
        size_t len = strlen(line);
        if (len && line[len - 1] == '\n') line[len - 1] = '\0';
        errno = 0;
        char *endp = NULL;
        long v = strtol(line, &endp, 10);
        if (endp == line || errno == ERANGE || *endp != '\0' || v < 1 || v > 3) {
            puts("Please enter 1, 2 or 3.");
            continue;
        }
        return (int)v;
    }
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    srand((unsigned)time(NULL));

    int mode = read_mode_choice();
    int difficulty = 1;
    if (mode == 2) difficulty = read_difficulty_choice();

    char board[ROWS][COLS];
    for (int i = 0; i < ROWS; i++) for (int j = 0; j < COLS; j++) board[i][j] = '.';

    char player = 'A';
    while (1) {
        print_board(board);
        int col;
        if (mode == 2 && player == 'B') {
            if (difficulty == 1)
                col = bot_choose_random_col(board);
            else if (difficulty == 2)
                col = bot_choose_medium_col(board, 'B', 'A');
            else
                col = bot_choose_impossible_col(board, 'B', 'A'); 

            if (col == -1) { printf("\nGame is a DRAW.\n"); break; }
                printf("\nBot (B) chooses column %d\n", col);
        } else {
            col = read_column_choice(player);
            if (col == -1) { printf("\nGoodbye!\n"); break; }
        }
        if (!drop_checker(board, col - 1, player)) {
            if (mode == 2 && player == 'B') continue;
            else { printf("Column %d is full. Pick a different column.\n", col); continue; }
        }
        if (is_winner(board, player)) {
            print_board(board);
            if (mode == 2 && player == 'B') printf("\nBot (B) WINS!\n");
            else printf("\nPlayer %c WINS!\n", player);
            break;
        }
        if (is_draw(board)) { print_board(board); printf("\nGame is a DRAW.\n"); break; }
        player = (player == 'A') ? 'B' : 'A';
    }
    return 0;
}
