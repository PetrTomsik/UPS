#ifndef _PLAYER_DB
#define _PLAYER_DB

#include "constants.h"
#include "game.h"

typedef struct PLAYER_DB{
    int amount_of_players;
    int max_amount_of_players;
    player **info;
} player_db;

extern player_db *global_db;
extern pthread_mutex_t db_lock;
 

int initiliaze_db();
int find_player(char *name);
int add_player(player *p);
player* get_player(char *name);
int remove_player(player *p);
void free_db();
#endif
