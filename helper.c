#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "helper.h"
#define INF 1000000000

void clear_screen(void) { }

void print_board(char board[ROWS][COLS]) {
    puts("");
    printf("  1 2 3 4 5 6 7\n");
    for (int i = 0; i < ROWS; i++) {
        printf("  ");
        for (int j = 0; j < COLS; j++) printf("%c ", board[i][j]);
        puts("");
    }
    fflush(stdout);
}

int drop_checker(char board[ROWS][COLS], int col, char token) {
    for (int i = ROWS - 1; i >= 0; i--) {
        if (board[i][col] == '.') { board[i][col] = token; return 1; }
    }
    return 0;
}

int check_direction(char board[ROWS][COLS], int r, int c, int dr, int dc, char token) {
    for (int i = 0; i < 4; i++) {
        int nr = r + dr * i, nc = c + dc * i;
        if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) return 0;
        if (board[nr][nc] != token) return 0;
    }
    return 1;
}

int is_winner(char board[ROWS][COLS], char token) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            if (board[r][c] == token &&
                (check_direction(board,r,c,0,1,token)||
                 check_direction(board,r,c,1,0,token)||
                 check_direction(board,r,c,1,1,token)||
                 check_direction(board,r,c,-1,1,token)))
                return 1;
    return 0;
}

int is_draw(char board[ROWS][COLS]) {
    for (int c = 0; c < COLS; c++) if (board[0][c] == '.') return 0;
    return 1;
}

int read_column_choice(char player) {
    char line[128];
    for (;;) {
        printf("\nPlayer %c, choose a column (1-7): ", player);
        if (!fgets(line, sizeof(line), stdin)) return -1;
        size_t len = strlen(line);
        if (len && line[len - 1] == '\n') line[len - 1] = '\0';
        errno = 0;
        char *endp = NULL;
        long val = strtol(line, &endp, 10);
        if (endp == line || errno == ERANGE || *endp != '\0') {
            printf("Invalid input. Please enter a number from 1 to 7.\n");
            continue;
        }
        if (val < 1 || val > 7) {
            printf("Invalid input. Please enter a number from 1 to 7.\n");
            continue;
        }
        return (int)val;
    }
}

static int first_free_row(char board[ROWS][COLS], int col0) {
    for (int r = ROWS - 1; r >= 0; r--) if (board[r][col0] == '.') return r;
    return -1;
}

static int wins_if_drop_here(char board[ROWS][COLS], int col0, char token) {
    int r = first_free_row(board, col0);
    if (r < 0) return 0;
    board[r][col0] = token;
    int ok = 0;
    int dirs[4][2] = {{0,1},{1,0},{1,1},{-1,1}};
    for (int d = 0; d < 4 && !ok; d++) {
        int dr = dirs[d][0], dc = dirs[d][1];
        for (int k = -3; k <= 0 && !ok; k++) {
            int sr = r + k*dr, sc = col0 + k*dc;
            if (check_direction(board, sr, sc, dr, dc, token)) ok = 1;
        }
    }
    board[r][col0] = '.';
    return ok;
}
static const int move_order[COLS] = {3, 2, 4, 1, 5, 0, 6};

static int evaluate_line(char a, char b, char c, char d, char bot, char opp) {
    int score = 0;
    char arr[4] = {a, b, c, d};
    int botCount = 0, oppCount = 0, emptyCount = 0;

    for (int i = 0; i < 4; i++) {
        if (arr[i] == bot) botCount++;
        else if (arr[i] == opp) oppCount++;
        else if (arr[i] == '.') emptyCount++;
    }

    if (botCount == 4) score += 100000;
    else if (botCount == 3 && emptyCount == 1) score += 100;
    else if (botCount == 2 && emptyCount == 2) score += 10;

    if (oppCount == 3 && emptyCount == 1) score -= 120;
    else if (oppCount == 2 && emptyCount == 2) score -= 10;

    return score;
}

static int score_position(char board[ROWS][COLS], char bot, char opp) {
    int score = 0;

    int center_col = COLS / 2;
    int centerCount = 0;
    for (int r = 0; r < ROWS; r++)
        if (board[r][center_col] == bot) centerCount++;
    score += centerCount * 3;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS - 3; c++) {
            score += evaluate_line(board[r][c], board[r][c+1],
                                   board[r][c+2], board[r][c+3],
                                   bot, opp);
        }
    }

    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r < ROWS - 3; r++) {
            score += evaluate_line(board[r][c], board[r+1][c],
                                   board[r+2][c], board[r+3][c],
                                   bot, opp);
        }
    }

    for (int r = 0; r < ROWS - 3; r++) {
        for (int c = 0; c < COLS - 3; c++) {
            score += evaluate_line(board[r][c], board[r+1][c+1],
                                   board[r+2][c+2], board[r+3][c+3],
                                   bot, opp);
        }
    }

    for (int r = 3; r < ROWS; r++) {
        for (int c = 0; c < COLS - 3; c++) {
            score += evaluate_line(board[r][c], board[r-1][c+1],
                                   board[r-2][c+2], board[r-3][c+3],
                                   bot, opp);
        }
    }

    return score;
}

static int search_game_tree(char board[ROWS][COLS], int depth, int maximizing,
                            char bot_token, char human_token, int alpha, int beta) {
    if (is_winner(board, bot_token))   return 1000000 + depth;  
    if (is_winner(board, human_token)) return -1000000 - depth; 
    if (is_draw(board) || depth == 0)  return score_position(board, bot_token, human_token);

    if (maximizing) {
        int best = -INF;
        for (int i = 0; i < COLS; i++) {
            int c = move_order[i];
            int r = first_free_row(board, c);
            if (r < 0) continue;
            board[r][c] = bot_token;
            int val = search_game_tree(board, depth - 1, 0, bot_token, human_token, alpha, beta);
            board[r][c] = '.';
            if (val > best) best = val;
            if (best > alpha) alpha = best;
            if (beta <= alpha) break; 
        }
        return best;
    } else {
        int best = INF;
        for (int i = 0; i < COLS; i++) {
            int c = move_order[i];
            int r = first_free_row(board, c);
            if (r < 0) continue;
            board[r][c] = human_token;
            int val = search_game_tree(board, depth - 1, 1, bot_token, human_token, alpha, beta);
            board[r][c] = '.';
            if (val < best) best = val;
            if (best < beta) beta = best;
            if (beta <= alpha) break; 
        }
        return best;
    }
}



int bot_choose_random_col(char board[ROWS][COLS]) {
    int candidates[COLS], n = 0;
    for (int c = 0; c < COLS; c++) if (board[0][c] == '.') candidates[n++] = c;
    if (n == 0) return -1;
    int pick = rand() % n;
    return candidates[pick] + 1;
}

int bot_choose_medium_col(char board[ROWS][COLS], char bot_token, char human_token) {
    for (int c = 0; c < COLS; c++)
        if (board[0][c] == '.')
            if (wins_if_drop_here(board, c, bot_token)) return c + 1;

    for (int c = 0; c < COLS; c++)
        if (board[0][c] == '.')
            if (wins_if_drop_here(board, c, human_token)) return c + 1;

    return bot_choose_random_col(board);
}


int bot_choose_impossible_col(char board[ROWS][COLS], char bot_token, char human_token) {
    int bestScore = -INF;
    int bestCol = -1;

    
    int searchDepth = 7;

    for (int i = 0; i < COLS; i++) {
        int c = move_order[i];
        if (board[0][c] != '.') continue;
        int r = first_free_row(board, c);
        if (r < 0) continue;

        board[r][c] = bot_token;
        int score = search_game_tree(board, searchDepth - 1, 0, bot_token, human_token, -INF, INF);
        board[r][c] = '.';

        if (score > bestScore) {
            bestScore = score;
            bestCol = c;
        }
    }

    if (bestCol == -1) return bot_choose_medium_col(board, bot_token, human_token);
    return bestCol + 1;
}
