#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "game.h"
#include "player.h"
#include "constants.h"
#include "message_handler.h"
#include "stack.h"
#include "player_db.h"
#include "lobby.h"

int id_counter = 1;
int game_id = 1;

stack *indexes_stack;

game **games;
int amount_of_games = 0, index_in_game_array;


void *lobby_handler(){
	int i;
	lobby *tmp_lobby;
	while(1){
		for(i = 0 ; i < amount_of_lobbies ; i++){
			tmp_lobby = list_of_lobbies[i];
			if(tmp_lobby->amount_of_players == AMOUNT_OF_PLAYERS){
				start_game(games, &amount_of_games, index_in_game_array, indexes_stack, list_of_lobbies[i]);
				remove_lobby(list_of_lobbies[i]->name);
				printf("Removed Ok!\n");
			}else if(tmp_lobby->amount_of_players == 0){
				remove_lobby(list_of_lobbies[i]->name);
			}
		}
	/*	print_database();
		printf("---------------\n");
		print_lobby();
*/
		sleep(2);

	}
	/*	printf("cheking! %d -> %d\n", is_game_startable(), global_lobby->amount_of_players);*/
		
}

int resize_game_array(game **g, int *max){
	if(!(*g)){
		printf("Game array empty!\n");
		return PARAMETER_INCORRECT;
	}

	(*max) *= RESIZING_CONSTANT;

	(*g) = realloc(*(g), *(max) * sizeof(game *)); 

	if(!(*g)){
		printf("Error while resizing array of games!\n");
		return RESIZING_UNSUCCESSFUL;
	}

	return RESIZING_SUCCESSFUL;
}

int main (int argc, char *argv[]){
	int server_sock, port, i;
	int client_socks;
	int return_value;
	char cbuf;
	char game_response[BUFFER_SIZE];
	char num_buf[BUFFER_SIZE];
	player *th_socket;
	struct sockaddr_in local_addr;
	struct sockaddr_in remote_addr;
	socklen_t remote_addr_len;
	pthread_t thread_id, game_id;
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	int max_amount_of_games = INITIAL_MAX_OF_GAME_ARRAY, new_game_flag = 1,
	resize_value, game_pointer = INITIAL_POSITION_IN_ARRAY, index_in_game_array;

	game *g;
	memset(&local_addr, 0, sizeof(struct sockaddr_in));

	local_addr.sin_family = AF_INET;

	/*if (server_sock<=0) return -1;*/
	switch(argc){
		case 3: local_addr.sin_addr.s_addr = inet_addr(argv[1]);
		for(i = 0 ; i < strlen(argv[2]) ; i++){
			if(!isdigit(argv[2][i])){
				printf("Invalid port!\n");
				return INVALID_PARAMETER;
			}
		}
		port = atoi(argv[2]);
		if(!port){
			printf("Port will be chosen randomly!\n");
		}

		if(port < 0 || port > 65535){ /* 65535 max value */
		
			return INVALID_PARAMETER;
		}
		
		local_addr.sin_port = htons(port);

		
		/*			printf("%s %s\n", argv[1], argv[2] );

			if(!local_addr.sin_port){
					local_addr.sin_port = htons(10001);
				}
				if(!local_addr.sin_addr.s_addr){
					local_addr.sin_addr.s_addr = INADDR_ANY;
				}

*/
				break;

		case 2: 
				local_addr.sin_port = htons(10001);
				local_addr.sin_addr.s_addr = inet_aton(argv[1]);
				break;
		default:local_addr.sin_port = htons(10001);
				local_addr.sin_addr.s_addr = INADDR_ANY;
				break;
	}
	printf("Settings !\n");

	return_value = bind(server_sock, (struct sockaddr *)&local_addr,\
                sizeof(struct sockaddr_in));

	if (return_value == 0)
		printf("Bind OK\n");
	else{
		printf("Bind ER\n");
		return -1;
	}

	return_value = listen(server_sock, 5);
	if (return_value == 0)
		printf("Listen OK\n");
	else{
		printf("Listen ER\n");
	}

	games = malloc(sizeof(game *));
	if(!games){
		printf("Unsuccessful games initialization!\n");
		return INITIALIZATION_UNSUCCESSFUL;
	}
	if(stack_create(&indexes_stack) < 0){
		free(games);
		printf("Unsuccessful stack initialization!\n");
		return INITIALIZATION_UNSUCCESSFUL;
	}

	if(initiliaze_db() == INITIALIZATION_UNSUCCESSFUL){
		free(games);
		free(indexes_stack);
		return INITIALIZATION_UNSUCCESSFUL;
	}

	if(initialize_lobby() == INITIALIZATION_UNSUCCESSFUL){
		free(games);
		free(indexes_stack);
		free_db();
		return INITIALIZATION_UNSUCCESSFUL;
	}


	pthread_create(&thread_id, NULL,\
                                  (void *)&lobby_handler, ((void*)th_socket));

	while(1){
		client_socks = accept(server_sock,\
			(struct sockaddr *)&remote_addr,\
			&remote_addr_len);
		if (client_socks > 0 ) {
/*
			th_socket=malloc(sizeof(player));
			th_socket->p = NULL;
			th_socket->socket_num = client_socks;
			printf("Creating a new thread: %d it:%d\n", client_socks, player_counter);
*/		/*	th_socket->p = malloc(sizeof(player)); */
			

			pthread_create(&thread_id, NULL,\
                                  (void *)&serve_player, ((void*)client_socks));
		/*	players[player_counter++] = th_socket;*/

			/*if(player_counter == AMOUNT_OF_PLAYERS){
				
				if((index_in_game_array = stack_pop(indexes_stack)) == EMPTY ){
					index_in_game_array = amount_of_games;
				
					if((amount_of_games == max_amount_of_games) && (new_game_flag)){
						resize_value = resize_game_array(&games, &max_amount_of_games);
						if(resize_value == RESIZING_UNSUCCESSFUL){
							printf("Unable to create a new game!\n");
							new_game_flag = 0;
						}
					}
				}
				

				player_counter = ANNULL_VALUE;
			}
*/
		} else {
			printf("Trable\n");
			return -1;
		}
	}


free(games);
stack_free(&indexes_stack);
return 0;
}
