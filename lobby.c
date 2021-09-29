#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "lobby.h"
#include "player.h"
#include "constants.h"

lobby **list_of_lobbies;
pthread_mutex_t lobby_lock;
int amount_of_lobbies = 0;
int max_amount_of_lobbies = MAX_AMOUNT_OF_LOBBIES;

int initialize_lobby(){
 /*   printf("Lobbies !\n");*/
    list_of_lobbies = malloc(sizeof(lobby *) * MAX_AMOUNT_OF_LOBBIES);
    /*printf("Lobbies test!\n");*/

    if(!list_of_lobbies){
        printf("Error while initializing lobby!\n");
        return INITIALIZATION_UNSUCCESSFUL;
    }

    if (pthread_mutex_init(&lobby_lock, NULL) != 0){
            printf("Error Mutex failed!\n");
            free((*list_of_lobbies));
            return INITIALIZATION_UNSUCCESSFUL;
    }

    return INITIALIZATION_SUCCESSFUL;
}

/*
int put_player_into_lobby(player *p){
    int amount;
    if(!p){
        printf("Error! Can't add null player!\n");
        return INVALID_PARAMETER;
    }

    amount = list_of_lobbies->amount_of_players;
    if(amount == list_of_lobbies->max_amount_of_players){
        if(resize_lobby < 0){
            printf("Error while resizing lobby!\n");
            return RESIZING_UNSUCCESSFUL;
        }
    }

    list_of_lobbies->players[amount] = p;
    list_of_lobbies->amount_of_players++;
    printf("Player put in a lobby!\n");
    return INITIALIZATION_SUCCESSFUL;
}

int resize_lobby(){
    lobby *n_lobby;
    int len = list_of_lobbies->max_amount_of_players + RESIZING_CONSTANT;
    
    n_lobby = malloc(sizeof(lobby) + len*sizeof(player *));
    if(!n_lobby){
        printf("Resizing unsuccessful - lobby !\n");
        return RESIZING_UNSUCCESSFUL;
    }

    list_of_lobbies = n_lobby;

    list_of_lobbies->max_amount_of_players = len;

    return INITIALIZATION_SUCCESSFUL;
}

int remove_from_lobby(player *p){
    char *p_name, *other_name;
    int i;

    if(!list_of_lobbies || !p || !p->name || list_of_lobbies->amount_of_players <= 0){
        printf("Error! Invalid parameter! - remove from lobby \n");
        return INVALID_PARAMETER;
    }
    strcpy(p_name, p->name);
    printf("Printing lobby names - remove: \n");
    for(i = 0 ; i < list_of_lobbies->amount_of_players ; i++){

        other_name = list_of_lobbies->players[i]->name;
        printf("player: %s\n", other_name);
        if(!strcmp(other_name, p_name)){
            printf("Removing player!\n");
            list_of_lobbies->players[i] = list_of_lobbies->players[list_of_lobbies->amount_of_players - 1];
            list_of_lobbies->players[list_of_lobbies->amount_of_players - 1] = NULL;
            list_of_lobbies->amount_of_players--;
            printf("Removed succesfully!\n");
            return REMOVED_SUCCESSFULLY;
        }
    }
}

void free_lobby(){
    int i;
    pthread_mutex_destroy(&lobby_lock);
    for(i = 0 ; i < (*list_of_lobbies)->amount_of_players ; i++){
        free((*list_of_lobbies)->players[i]);
    }
    free((*list_of_lobbies)->players);
    free((*list_of_lobbies));
}


player* get_from_lobby(){
    int i;
    player *p;
    if(!list_of_lobbies || list_of_lobbies->amount_of_players <= 0 ){
        printf("Invalid param! get_from_lobby \n");
        return NULL;
    }
    printf("Lobby last one : %s\n", list_of_lobbies->players[list_of_lobbies->amount_of_players - 1]->name);    
   return list_of_lobbies->players[list_of_lobbies->amount_of_players - 1]; /* Last index is amount - 1 
}*/


void print_lobby(){
    pthread_mutex_lock(&lobby_lock);

    int i, j;
    printf("Printing lobby!\n\n");
    for(i = 0 ; i < amount_of_lobbies ; i++){
        printf("%d.: %s ",i, list_of_lobbies[i]->name);

        for(j = 0 ; j < list_of_lobbies[i]->amount_of_players ; j++){
            printf("\t%d Player: %s %d \n", j, list_of_lobbies[i]->players[j]->name, list_of_lobbies[i]->players[j]);
        } 
    }
    printf("\n\n");
    pthread_mutex_unlock(&lobby_lock);

}



/*
int is_game_startable(){
    return list_of_lobbies->amount_of_players >= 2;    
}
*//* 2 players are needed for the game */

int create_lobby(char *name, player *p){    
    pthread_mutex_lock(&lobby_lock);
 /*   printf("Creating lobby!\n");*/
    lobby *n_lobby;
    if(!strlen(name) || !p){
        printf("Invalid params! - create lobby\n");
        pthread_mutex_unlock(&lobby_lock);
        return INVALID_PARAMETER;
    }

    if(amount_of_lobbies == max_amount_of_lobbies){
        printf("Max lobbies !\n");
            pthread_mutex_unlock(&lobby_lock);

        return ERR_MAX_LOBBY;
    }
    pthread_mutex_unlock(&lobby_lock);

    if(find_lobby(name) < 0 ){
        printf("Lobby exists !\n");
        pthread_mutex_unlock(&lobby_lock);
        return ERR_LOBBY_EXISTS;
    }
    pthread_mutex_lock(&lobby_lock);

 /*   printf("Checking state !\n");*/
    if(p->state == IN_LOBBY && strlen(p->curr_lobby)){
        /*printf("Changing lobby from %s -> %s! \n",p->curr_lobby, name);*/
        pthread_mutex_unlock(&lobby_lock);
        remove_from_lobby(p->curr_lobby, p);
        pthread_mutex_lock(&lobby_lock);

    }
 /*   printf("Checking state !\n");*/

    n_lobby = malloc(sizeof(lobby));
    if(!n_lobby){
        printf("lobby init unsuccessful!\n");
            pthread_mutex_unlock(&lobby_lock);

        return INITIALIZATION_UNSUCCESSFUL;
    }
 /*   printf("Checked state !\n");*/

    strcpy(n_lobby->name, name);
    strcpy(p->curr_lobby, name);
    list_of_lobbies[amount_of_lobbies++] = n_lobby;

    pthread_mutex_unlock(&lobby_lock);
    if(join_lobby(name, p) < 0){
        pthread_mutex_lock(&lobby_lock);
        amount_of_lobbies--;
        free(list_of_lobbies[amount_of_lobbies]);
        list_of_lobbies[amount_of_lobbies] = NULL;
        pthread_mutex_unlock(&lobby_lock);
        return INITIALIZATION_UNSUCCESSFUL;
    }
        pthread_mutex_lock(&lobby_lock);

     /*   printf("Checking state !\n");*/

    p->state = IN_LOBBY;
    pthread_mutex_unlock(&lobby_lock);
    return INITIALIZATION_SUCCESSFUL;
}

int find_lobby(char *name){
        pthread_mutex_lock(&lobby_lock);


    int i;
    lobby *tmp_lobby;

      for(i = 0 ; i < amount_of_lobbies ; i++ ){
        tmp_lobby = list_of_lobbies[i];
  /*      printf("Comparing %s and %s\n", tmp_lobby->name, name);*/
        if(!strcmp(tmp_lobby->name, name)){
            pthread_mutex_unlock(&lobby_lock);

            return ERR_LOBBY_EXISTS;
        }
    }
            pthread_mutex_unlock(&lobby_lock);

    return LOBBY_NOT_FOUND;
}


int join_lobby(char *name, player *p){
    pthread_mutex_lock(&lobby_lock);
 /*   printf("Joining lobby!\n");*/

    int i;
    lobby *tmp_lobby;
    if(!name || !p){
        printf("Invalid params! - join lobby\n");
        pthread_mutex_unlock(&lobby_lock);

        return INVALID_PARAMETER;
    }
  /*  printf("Lobby %s\n", p->curr_lobby);*/
    if(strlen(p->curr_lobby)){
        pthread_mutex_unlock(&lobby_lock);
        remove_from_lobby(p->curr_lobby, p);
        pthread_mutex_lock(&lobby_lock);
    }

    for(i = 0 ; i < amount_of_lobbies ; i++ ){
        tmp_lobby = list_of_lobbies[i];
    /*    printf("Lobby Comparing %s and %s\n", tmp_lobby->name, name);*/
        if(!strcmp(tmp_lobby->name, name)){
            if(tmp_lobby->amount_of_players == AMOUNT_OF_PLAYERS){
                printf("Can't join lobby !\n");
                pthread_mutex_unlock(&lobby_lock);
                return ERR_LOBBY_FULL;
            }
            if(tmp_lobby->amount_of_players){
                if(!strcmp(tmp_lobby->players[0]->name, p->name)){
                    printf("User is already in lobby!\n");
                    pthread_mutex_unlock(&lobby_lock);
                    return ERR_PLAYER_ALREADY_IN_LOBBY;
                }
            }

            p->state = IN_LOBBY;
      /*      printf("Adding player to lobby %s %d %d \n",p->name , tmp_lobby->amount_of_players, p);*/
            tmp_lobby->players[tmp_lobby->amount_of_players] = p;
            tmp_lobby->amount_of_players++;
            strcpy(p->curr_lobby, name);
            pthread_mutex_unlock(&lobby_lock);

            return LOBBY_JOINED;
        }

    }

    pthread_mutex_unlock(&lobby_lock);

    return ERR_NO_SUCH_LOBBY;
}

int remove_from_lobby(char *name, player *p){
    pthread_mutex_lock(&lobby_lock);

 /*   printf("Removing from lobby %d !\n", amount_of_lobbies);*/
    lobby *tmp_lobby;
        int i;

    if(!name || !p){
        printf("Invalid params! - remove from lobby\n");
        pthread_mutex_unlock(&lobby_lock);

        return INVALID_PARAMETER;
    }

    for(i = 0 ; i < amount_of_lobbies ; i++ ){
        tmp_lobby = list_of_lobbies[i];
 /*       printf("Comparing %s and %s\n", tmp_lobby->name, name);*/
        if(!strcmp(tmp_lobby->name, name)){
            /* Player was removed already */
            if(!tmp_lobby->players[0]){
                break;
            }
            /* Is it first player? */
            if(!strcmp(tmp_lobby->players[0]->name, p->name)){
                memset(tmp_lobby->players[0]->curr_lobby, 0, LOBBY_NAME);
                tmp_lobby->players[0] = tmp_lobby->players[1];
                tmp_lobby->players[1] = NULL;
                
                /* Is it second player? */
          /*  }else if(!strcmp(tmp_lobby->players[1]->name, p->name)){
                memset(tmp_lobby->players[1]->curr_lobby, 0, LOBBY_NAME);
                tmp_lobby->players[1] = NULL;*/
            }else{
                break; /* Neither of the players */
            }

            tmp_lobby->amount_of_players--;

            pthread_mutex_unlock(&lobby_lock);
            if(!tmp_lobby->amount_of_players){
                remove_lobby(name);
            }
            
            return REMOVED_SUCCESSFULLY;
        }

    }
    pthread_mutex_unlock(&lobby_lock);

    return REMOVED_UNSUCCESSFULLY;
}

char* get_lobbies(){
    pthread_mutex_lock(&lobby_lock);
  /*  printf("Getting lobbies !\n");*/

    int i, offset = 10;
    lobby *temp_lobby;
    if(!amount_of_lobbies){
        pthread_mutex_unlock(&lobby_lock);
        return NULL;
    }
    char *buffer;
    buffer = malloc(sizeof(char) * (amount_of_lobbies * LOBBY_NAME + offset));
    if(!buffer){
        printf("Error while creating lobbies!\n");
        pthread_mutex_unlock(&lobby_lock);
        return NULL;
    }

    memset(buffer, 0, amount_of_lobbies * LOBBY_NAME + offset);
    for(i = 0 ; i < amount_of_lobbies - 1 ; i++){
        temp_lobby = list_of_lobbies[i];
        sprintf(buffer, "%s%s-%d/2,",buffer, temp_lobby->name, 
        temp_lobby->amount_of_players);
    }
 /*   printf("Lobbies: %d\n", amount_of_lobbies);*/
    temp_lobby = list_of_lobbies[amount_of_lobbies-1];
    sprintf(buffer, "%s%s-%d/2",buffer, temp_lobby->name, 
    temp_lobby->amount_of_players);

    pthread_mutex_unlock(&lobby_lock);

    return buffer;
}


int remove_lobby(char *name){
    pthread_mutex_lock(&lobby_lock);


   lobby *tmp_lobby;
       int i;

    if(!name){
        printf("Invalid params! - remove from lobby\n");
        pthread_mutex_unlock(&lobby_lock);

        return INVALID_PARAMETER;
    }

    for(i = 0 ; i < amount_of_lobbies ; i++ ){
        tmp_lobby = list_of_lobbies[i];
/*        printf("Comparing %s and %s on index %d\n", tmp_lobby->name, name,i);*/
        if(!strcmp(tmp_lobby->name, name)){
            free(list_of_lobbies[i]);
            list_of_lobbies[i] = list_of_lobbies[amount_of_lobbies-1];
            list_of_lobbies[amount_of_lobbies-1] = NULL;
            amount_of_lobbies--;
            pthread_mutex_unlock(&lobby_lock);

            return REMOVED_SUCCESSFULLY;
        }

    }
    pthread_mutex_unlock(&lobby_lock);

    return REMOVED_UNSUCCESSFULLY;

}
