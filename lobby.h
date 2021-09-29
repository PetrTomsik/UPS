#ifndef _LOBBY_H
#define _LOBBY_H

#include <pthread.h>

#include "player.h"
#include "constants.h"


typedef struct LOBBY{
    struct PLAYER *players[AMOUNT_OF_PLAYERS];
    char name[LOBBY_NAME];
    int max_amount_of_players;
    int amount_of_players;
}lobby;

extern lobby **list_of_lobbies;
extern pthread_mutex_t lobby_lock;
extern int amount_of_lobbies;

int initialize_lobby();

int create_lobby(char *name, struct PLAYER *p);
int join_lobby(char *name, struct PLAYER *p);
int resize_lobby();
int remove_from_lobby(char *name, struct PLAYER *p);
int remove_lobby(char *name);
int find_lobby(char *name);
char* get_lobbies();

int put_player_into_lobby(struct PLAYER *p);
struct PLAYER* get_from_lobby();
int is_game_startable();
void print_lobby();
#endif