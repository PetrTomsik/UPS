#ifndef _PLAYER_H
#define _PLAYER_H

#include "game.h"
#include "constants.h"

/**
 * @brief Structure representing player
 * 
 */
typedef struct PLAYER{
    int state;
    int game_id;
    int player_id;
    int order;
    char name[PLAYER_NAME];
    char curr_lobby[LOBBY_NAME];
    int socket_num;
    int other_sock; /* Reconsider changing into one structure */ 
    struct GAME *g;
    int pinged;
    int victories;
    int loses;
}player;


void *serve_player(void *arg);
int *handle_ping(void *arg);
int forcefully_end_game(player *p, struct GAME *g);
#endif