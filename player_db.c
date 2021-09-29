#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "constants.h"
#include "player_db.h"
#include "lobby.h"

player_db *global_db;
pthread_mutex_t db_lock;

int initiliaze_db(){
    global_db = malloc(sizeof(player_db));

    if(!global_db){
        printf("Error while initializing database!\n");
        return INITIALIZATION_UNSUCCESSFUL;
    }
    global_db->info = malloc(sizeof(player*));

    if(!global_db->info){
        pthread_mutex_destroy(&db_lock);
        free(global_db);
        printf("Error while initializing database! - info \n");
        return INITIALIZATION_UNSUCCESSFUL;
    }

    if (pthread_mutex_init(&db_lock, NULL) != 0){
        free(global_db->info);
        free(global_db);
        printf("Error Mutex failed!\n");
        return INITIALIZATION_UNSUCCESSFUL;
    }

    return INITIALIZATION_SUCCESSFUL;
}


int find_player(char *name){
    pthread_mutex_lock(&db_lock);
    int i;
    char temp_name[PLAYER_NAME];
    /*    printf("Testing %d !\n",global_db->amount_of_players );*/

    if(!global_db || !name ){
        printf("Error invalid parameter! - find player\n");
        pthread_mutex_unlock(&db_lock);

        return INVALID_PARAMETER;
    }
 /*   printf("Searching !\n");*/
    for(i = 0 ; i < global_db->amount_of_players; i++ ){
        strcpy(temp_name, global_db->info[i]->name);
        /*printf("Comparing %s and %s", temp_name, name);*/
        if(!strcmp(name, temp_name)){   
            pthread_mutex_unlock(&db_lock);
            return PLAYER_FOUND;
        }
    }

    pthread_mutex_unlock(&db_lock);

    return PLAYER_NOT_FOUND;    /* Player isn't in the database */ 
}

int add_player(player *p){
    pthread_mutex_lock(&db_lock);


    player_db *db;
    int amount;
    int len;

    if(!p || !global_db){
        pthread_mutex_unlock(&db_lock);
        return INVALID_PARAMETER;
    }
    len = RESIZING_CONSTANT + amount;
    amount = global_db->amount_of_players;

    if(global_db->max_amount_of_players == amount){ /* Need to check case where theres only one space left */
        db = realloc(global_db, sizeof(player_db) + len* sizeof(player) );
        if(!db){
            printf("Resizing unsuccessful! - add players \n");
            pthread_mutex_unlock(&db_lock);

            return RESIZING_UNSUCCESSFUL;
        }
        global_db = db;
        global_db->max_amount_of_players = len;

    }

    global_db->info[amount] = p;
    global_db->amount_of_players++;
   /* printf("\n\nPlayer added! %d \n\n", global_db->amount_of_players);*/
    
    pthread_mutex_unlock(&db_lock);

    return INITIALIZATION_SUCCESSFUL;
}

int remove_player(player *p){

    pthread_mutex_lock(&db_lock);

    int i;
    char temp_name[PLAYER_NAME];

    if(!global_db || !p || global_db->amount_of_players == EMPTY){
        printf("Error invalid parameter! - find player\n");
        pthread_mutex_unlock(&db_lock);
        
        return INVALID_PARAMETER;
    }
    
    for(i = 0 ; i < global_db->amount_of_players; i++ ){
        strcpy(temp_name, global_db->info[i]->name);
        if(!strcmp(p->name, temp_name)){
            free(global_db->info[i]);
            /* So space isn't wasted */
            global_db->info[i] = global_db->info[global_db->amount_of_players-1];
            global_db->info[global_db->amount_of_players] = NULL;
            global_db->amount_of_players--;
    pthread_mutex_unlock(&db_lock);

            return REMOVED_SUCCESSFULLY;
            
        }
    }


    pthread_mutex_unlock(&db_lock);

    return REMOVED_UNSUCCESSFULLY;    /* Player isn't in the database, shouldn't happen */ 
}

void free_db(){
    int i;
    for(i = 0 ; i < global_db->amount_of_players ; i++){
        free(global_db->info[i]);
    }
    free(global_db->info);
    free(global_db);
    pthread_mutex_destroy(&db_lock);
}

void print_database(){
    pthread_mutex_lock(&db_lock);
    int i;
    printf("Printing database! %d\n\n", global_db->amount_of_players);
    for(i = 0 ; i < global_db->amount_of_players ; i++){
        if(!global_db->info[i]){
            continue;
        }
        printf("%d.: %s\n",i, global_db->info[i]->name);
    }
    pthread_mutex_unlock(&db_lock);

}

player* get_player(char *name){
    pthread_mutex_lock(&db_lock);

    int i;
    if(!name || !strlen(name)){
        printf("Invalid params !- getplayer\n");
        pthread_mutex_unlock(&db_lock);

        return INVALID_PARAMETER;
    }

    for(i = 0 ; i < global_db->amount_of_players; i++ ){
        if(!strcmp(global_db->info[i]->name, name)){   
            pthread_mutex_unlock(&db_lock);
     
            return global_db->info[i];
        }
    }

    pthread_mutex_unlock(&db_lock);

    return NULL;    /* Shouldn't happen */
}
