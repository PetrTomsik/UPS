#ifndef _GAME_H
#define _GAME_H

#define SIZE_OF_BOARD 10
#define AMOUNT_OF_PLAYERS 2

#define GAMEBOARD short[][] gameBoard = new short[SIZE_OF_BOARD][SIZE_OF_BOARD];
/*#define short counter[] = new short[SIZE_OF_BOARD];
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "stack.h"
#include "player.h"
#include "lobby.h"

typedef short int data;

typedef struct GAME{
    int id;
    int index;
    int state; /* state of the server/game */
    int amount_left; /* how many blank spaces are left */
    data **board;   /* state of the game board  */
    data *pointers; /* Player cannot send Y coordinate, server covers that for him */
    stack *stck;
    struct PLAYER *players[2]; /* players */
}game;

/*int initialize_game(game **f_game, struct SOCKET_INFO *sock1, struct SOCKET_INFO *sock2, stack *stck, int index);*/
int check_move(char *move);
void free_game(game **f_game);
int change_state();
int initialize_game(game **f_game, struct LOBBY *n_lobby, stack *stck, int index);
void start_game(game **games, int *amount_of_games, int index, stack *stck, struct LOBBY *n_lobby);
void end_game(game *g);
char *get_state(game *g, int order);
/*void start_game(struct SOCKET_INFO **players, game **games, int *amount_of_games);*/
#endif