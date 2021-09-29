#include <stdlib.h>

#include "constants.h"
#include "player.h"
#include "game.h"
#include "player_db.h"
#include "lobby.h"

int game_id_counter = 1;
/*
int initialize_game(game **f_game, struct player *sock1, struct player *sock2, stack *stck, int index);
void start_game(struct player **players, game **games, int *amount_of_games, int index, stack *stck);
*/
int initialize_game(game **f_game, struct LOBBY *n_lobby, stack *stck, int index){
    game *new_game;
    player *new_players[2], *sock1, *sock2;
    int i,j;

    if(!stck || !n_lobby){
        printf("Invalid param ! - init game\n");
        return INVALID_PARAMETER;
    }

    
    new_game = malloc(sizeof(game));
    new_game -> state = INITIALIZING;

    /* Game initialization */
    if(!new_game){
        printf("Error initializing the game! \n");
        return INITIALIZATION_UNSUCCESSFUL;
    }
  /*  printf("Game initialized!\n");*/
    new_game->pointers = calloc(SIZE_OF_BOARD, sizeof(data));

    if(!new_game->pointers){
        free(new_game);
        printf("Error initializing the pointers! \n");
        return INITIALIZATION_UNSUCCESSFUL;
    }
    new_game->amount_left = SIZE_OF_BOARD * SIZE_OF_BOARD;


    new_game -> board = malloc(sizeof(data*) * SIZE_OF_BOARD);
    if(!new_game->board){
        free(new_game->pointers);
        free(new_game);
        printf("Error initializing the matrice! \n");
        return INITIALIZATION_UNSUCCESSFUL;
    }
    /*    printf("Pointers initialized!\n");*/

    for( i = 0 ; i < SIZE_OF_BOARD ; i++){
        new_game->board[i] = calloc(SIZE_OF_BOARD, sizeof(data));
        /* Initialization failed */
        if(!(new_game->board[i])){
            for( j = i ; j >= 0 ; j--){
                free(new_game->board[j]);
            }
            free(new_game->board);
            free(new_game->pointers);
            free(new_game);

            printf("Error while initializing board\n");
            return INITIALIZATION_UNSUCCESSFUL;
        }
    }
    sock1 = n_lobby->players[0];
    sock2 = n_lobby->players[1];

    sock1->order = PLAYER1_TURN;
    sock2->order = PLAYER2_TURN;
    
    new_game->id = game_id_counter++;
    new_game->stck = stck;
    new_game->index = index;

    /* Players assertion to game */
    (sock1)->game_id = new_game->id;
    (sock2)->game_id = new_game->id;

    /* To send messages to other player */
    (sock1)->other_sock = (sock2)->socket_num;
    (sock2)->other_sock = (sock1)->socket_num;

    (sock1)->order = PLAYER1_TURN;
    (sock2)->order = PLAYER2_TURN;

    new_players[0] = sock1;
    new_players[1] = sock2;
    

    *(new_game->players) = (*new_players);

    new_game->state = PLAYER1_TURN;

    *f_game = new_game;
/*    printf("Players initialized!\n");*/


    return INITIALIZATION_SUCCESSFUL;
}


void start_game(game **games, int *amount_of_games, int index, stack *stck, struct LOBBY *n_lobby){
    char game_response[BUFFER_SIZE];
    game *g;
    player *players[AMOUNT_OF_PLAYERS];
    /* Game initialization */

/*    printf("Testing! stck:%d, games:%d\n",stck, games[*amount_of_games]);*/
    if(!games || !stck ){
        printf("Incorrect params!\n");
        return;
    }


    if(initialize_game(&g, n_lobby, stck, index) < 0){
        printf("Error while initializing game!\n");
        clear_buffer(&game_response);
        sprintf(game_response, "Error while initializing game!\n\r");
        send(players[0]->socket_num, &game_response, sizeof(game_response), 0);
        send(players[1]->socket_num, &game_response, sizeof(game_response), 0);
        return;
    }
  /*  printf("Removing from lobby!%d %d\n", n_lobby->players[0], n_lobby->players[1]);*/
    players[0] = n_lobby->players[0];
    players[1] = n_lobby->players[1];
/*    games[*amount_of_games] = g;*/

 /*  add_players(g);*/
  /*  printf("\n\nGame init! %d p1:%s p2:%s\n", g, players[0]->name, players[1]->name);*/

    players[0]->g = g;
    players[1]->g = g;

    players[0]->state = IN_GAME;
    players[1]->state = IN_GAME;

   /* printf("Game response!\n");*/
    clear_buffer(&game_response);
    sprintf(game_response, "%d|GAME_ID|%d\n\r", GAME_START,g->id);

    /*printf("Game response: %s\n", game_response);*/

   /* (*amount_of_games)++;*/
  /*  printf("Sending response: %s %d to %d and %d\n", game_response, g->id, players[0]->socket_num, players[1]->socket_num);*/
    g->players[0] = players[0];
    g->players[1] = players[1];

    send(players[0]->socket_num, &game_response, sizeof(game_response), 0);
    send(players[1]->socket_num, &game_response, sizeof(game_response), 0);
    
    clear_buffer(&game_response);
}

void free_game(game **f_game){
    int i;

    if(!(*f_game)){
        return;
    }
/*
    for(i = 0 ; i < SIZE_OF_BOARD ; i++ ){
        free((*f_game)->board[i]);
    }
    printf("freeing board!\n");
    free((*f_game)->board);
    printf("freeing pointers!\n");
    free((*f_game)->pointers);
    printf("freeing game!\n");
    free((*f_game));
    (*f_game) = NULL;*/
}

void end_game(game *g){
    if(!g){
        return;
    }

    printf("Ending game!\n");
    if(g->players[0]){
        g->players[0]->state = IN_LOBBY;

    }

    if(g->players[1]){
        g->players[1]->state = IN_LOBBY;
    }

  
    free_game(&g);

}


char *get_state(game *g, int order){
    char *state, buf[9];    /* 8B + '0' */
    
    int i, j, offset = 64;   /* Space for \n\r and some other things */
    if(!g){
        printf("Invalid parameter! - game get state \n");
        return INVALID_PARAMETER;
    }
    /* Format pl(1B)-(1B)x_coord(1-2B),(1B)y_coord(1-2B)' '(1B)... - 8 Bytes for one state*/
    state = malloc(sizeof(char) * 8 * SIZE_OF_BOARD * SIZE_OF_BOARD + offset);
    if(!state){
        printf("State not initialized!\n");
        return INITIALIZATION_UNSUCCESSFUL;
    }
    
    printf("Printing state ! - get_state\n");
    memset(buf, 0, sizeof(buf));
    memset(state, 0, sizeof(state));

    sprintf(state, "%d|%d|", RECONNECTION_SUCCESFUL, order, state);

    for(i = 0 ; i < SIZE_OF_BOARD ; i++){
        for(j = 0 ; j < SIZE_OF_BOARD  ; j++){
            sprintf(buf, "%d-%d,%d ", g->board[i][j], i, j);
            /*printf("Conc: %s\n", buf);*/
            sprintf(state, "%s%s",state, buf);

            if(g->board[i][j]){
              /*  printf("move: %d\n", g->board[i][j]);*/
            }
        }
    }
/*    printf("State: %s\n", state);*/
    state[strlen(state)-1] = '\0';
    sprintf(state, "%s|%s\n\r", state, "Reconnected");

 /*   printf("State: %s\n", state);*/
    return state;
}

