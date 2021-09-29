#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <ctype.h>

#include "message_handler.h"
#include "player.h"
#include "constants.h"
#include "player_db.h"
#include "lobby.h"

int id_player_counter = 1;

/**
 * @brief Handles message from client
 * 
 * @param client_sock client number
 * @param message message
 * @param p which player sent the message
 * @return int success / failure, depending on the message (details in protocol)
 */
int handle_message(int client_sock, char *message, player **p, int other_sock){
	int reconnect, pl_id, move_x, move_y, result, i;
	char *splitted, *pl_turn, errbuf[BUFFER_SIZE], infobuf[BUFFER_SIZE], num_buf[BUFFER_SIZE];
	player *sock = *p;
	pthread_t thread_id;

	if(!message || !strlen(message)){
		return MESSAGE_NOK;
	}
	clear_buffer(&errbuf);
	clear_buffer(&infobuf);
	clear_buffer(&num_buf);

	splitted = strtok(message, DELIMITER);

	printf("Message splitted: %s!\n", splitted);
	if(!splitted || !strlen(splitted)){
		clear_buffer(&errbuf);
		sprintf(errbuf,"%d|ERROR! Invalid message!\n\r", MESSAGE_NOK);
		send(client_sock, &errbuf, sizeof(errbuf), 0);
		return MESSAGE_NOK;
	}
	/* connect|pl_id|name	*/
	if(strcmp(splitted, "connect") == 0){
		printf("Connecting...\n");

		splitted = strtok(NULL, DELIMITER);
		printf("Message splitted again! %s\n", splitted);
		for(i = 0 ; i < strlen(splitted) ; i++){
			if(!isdigit(splitted[i])){
				clear_buffer(&errbuf);
				sprintf(errbuf, "%d|Error! Invalid pl_id!\n", ERR_PLAYER_TURN);
				send(client_sock, &errbuf, sizeof(errbuf), 0);
				return INVALID_PARAMETER;
			}
		}
		if(!splitted || !strlen(splitted)){
			clear_buffer(&errbuf);
			sprintf(errbuf,"%d|ERROR! Invalid message!\n\r", MESSAGE_NOK);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return MESSAGE_NOK;
		}

		printf("Creating a new player!\n");
		splitted = strtok(NULL, DELIMITER);
		printf("Naming a player %d!\n", sock);

		if(!splitted || !strlen(splitted)){
			clear_buffer(&errbuf);
			sprintf(errbuf,"%d|ERROR! Invalid message!\n\r", MESSAGE_NOK);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return MESSAGE_NOK;
		}

		for(i = 0 ; i < strlen(splitted) ; i++){
			if(!isalpha(splitted[i])){
				clear_buffer(&errbuf);
				sprintf(errbuf, "%d|Error! Invalid name!\n", ERR_INVALID_NAME);
				send(client_sock, &errbuf, sizeof(errbuf), 0);
				return INVALID_PARAMETER;
			}
		}
		if(!splitted || !strlen(splitted)){
			clear_buffer(&errbuf);
			sprintf(errbuf,"%d|ERROR! Invalid message!\n\r", MESSAGE_NOK);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return MESSAGE_NOK;
		}
		/* Name too long */
		if(strlen(splitted) >= PLAYER_NAME){
			clear_buffer(&errbuf);
			sprintf(errbuf,"%d|ERROR! Name too long!\n\r", ERR_LONG_NAME);
			/*strcpy(errbuf,"|ERROR! Name too long!\n\r");*/
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			clear_buffer(&errbuf);
			return INVALID_PARAMETER;
		}
		printf("Searching for player!\n");
		/* Name was found */
		if(find_player(splitted) < 0){
			
			sock = get_player(splitted);
			printf("Player found! %s \n", sock->name);
			if(!sock){
				printf("Error! User isn't in database!\n");
				clear_buffer(&errbuf);
				sprintf(errbuf,"%d|Error! User isn't in database!\n\r", ERR_USER_NOT_IN_DB);
				send(client_sock, &errbuf, sizeof(errbuf), 0);
				return INITIALIZATION_UNSUCCESSFUL;
			}
			if(sock->state == IN_LOBBY || sock->state == IN_GAME){
				printf("User with the same name found! Sending a response\n");
				clear_buffer(&errbuf);
				sprintf(errbuf,"%d|ERROR! User exists!\n\r", ERR_USER_EXISTS);
				send(client_sock, &errbuf, sizeof(errbuf), 0);
				clear_buffer(&errbuf);
				return INVALID_PARAMETER;
			}else if(sock->state == DISCONNECTED){
				char *state;
				
				sock->socket_num = client_sock;
					printf("Setting up socket! %d %d\n", sock->g->players[0], sock->g->players[1]);

				if(strcmp(sock->name, sock->g->players[0]->name)){
					sock->g->players[0]->other_sock = client_sock;	/* Can't send other messages otherwise */
				}else{
					sock->g->players[1]->other_sock = client_sock;
				}
				*p = sock;
				clear_buffer(&infobuf);
				state = get_state(sock->g, sock->order);
				send(client_sock, state, strlen(state), 0);
				free(state);
				sock->state = IN_GAME;	 /* If player was in lobby, he would be removed from db */
				return RECONNECTION_OK;
			}
			
		}
		printf("Not found!\n");

		/* New player */
		sock = malloc(sizeof(player));
		if(!sock){
			printf("Error initializating a player!\n");
			return INITIALIZATION_UNSUCCESSFUL;
		}
		sock->player_id = 0;
		strcpy(sock->name, splitted);
		/*  */
        if(!(sock->name)){
			free(sock);
            printf("Error while initializing name!\n");
            return INITIALIZATION_UNSUCCESSFUL;
        }
		/* Adding player failed */
		if(add_player(sock) < 0){
			free(sock->name);
			free(sock);
			clear_buffer(&errbuf);
			sprintf(infobuf,"%d|ERROR! User couldn't be created! Try it later.\n\r", ERR_PLAYER_INIT_UNSUCCESSFUL);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			clear_buffer(&errbuf);
			return INITIALIZATION_UNSUCCESSFUL;
		}
		printf("Creating a response!\n");
	
		printf("Name of player: %s, %s lobby %s , pointer: %d\n", sock->name, infobuf, sock, sock->curr_lobby);
		clear_buffer(&infobuf);
		sprintf(infobuf, "205|%d|Connected\n\r", id_player_counter);
		send(client_sock, &infobuf, sizeof(infobuf), 0);

		sock->player_id = id_player_counter++;
		sock->socket_num = client_sock;
		sock->other_sock = -1;
		sock->state = IN_LOBBY;
		sock->pinged = PING_OK;
		memset(sock->curr_lobby, 0, LOBBY_NAME);
		*p = sock;
		pthread_create(&thread_id, NULL,\
                                  (void *)&handle_ping, ((void*)sock));
	/*	players[player_counter++] = sock;*/
		clear_buffer(&infobuf);
		return INITIALIZATION_SUCCESSFUL;

	}else if(*p == NULL){
		clear_buffer(&errbuf);
		sprintf(errbuf,"Error! Please connect with your name and then send other messages!\n\r");
		send(client_sock, &errbuf, sizeof(errbuf), 0);
		return MESSAGE_NOK;
	}else if(strcmp(splitted, "move") == 0){
		/* Move message - Expected format "move|<x,y>|<player_id>"*/ 

		splitted = strtok(NULL, DELIMITER);	 /* <x,y> */
		pl_turn = strtok(NULL, DELIMITER); /* player_id */
		splitted = strtok(splitted, ",");
		if(!splitted || !strlen(splitted)){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Invalid x-coord!\n", ERR_X_COORD);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INVALID_PARAMETER;
		}
		printf("Checking x-coord %d %c!\n", strlen(splitted), splitted[0]);
		for(i = 0 ; i < strlen(splitted) ; i++){
			if(!isdigit(splitted[i])){
				clear_buffer(&errbuf);
				sprintf(errbuf, "%d|Error! Invalid x-coord!\n", ERR_X_COORD);
				send(client_sock, &errbuf, sizeof(errbuf), 0);
				return INVALID_PARAMETER;
			}
			printf("i: %d X:%c\n", i, splitted[i]);
		}
		printf("checking done!\n");
		move_x = atoi(splitted);
		printf("checking player turn ! %d\n", strlen(pl_turn));
		if(!pl_turn || !strlen(pl_turn)){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! No player ID!\n", ERR_NO_ID);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INVALID_PARAMETER;
		}
		printf("checking player id done!\n");

		splitted = strtok(NULL, ",");
		
		if(!splitted || !strlen(splitted)){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Invalid y-coord!\n", ERR_Y_COORD);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INVALID_PARAMETER;
		}

		for(i = 0 ; i < strlen(pl_turn) ; i++){
			if(!isdigit(pl_turn[i])){
				clear_buffer(&errbuf);
				sprintf(errbuf, "%d|Error! Invalid player turn!\n", ERR_PLAYER_TURN);
				send(client_sock, &errbuf, sizeof(errbuf), 0);
				return INVALID_PARAMETER;
			}
		}
		printf("Checking y-coord!\n");

		for(i = 0 ; i < strlen(splitted) ; i++){
			if(!isdigit(splitted[i])){
				clear_buffer(&errbuf);
				sprintf(errbuf, "%d|Error! Invalid y-coord!\n", ERR_Y_COORD);
				send(client_sock, &errbuf, sizeof(errbuf), 0);
				return INVALID_PARAMETER;
			}
		}
		move_y = atoi(splitted);
		other_sock = sock->other_sock;
		printf("Splitted ok !\n");
		result = handle_move_message(client_sock, sock->g, sock, move_x, move_y, other_sock);		
		return result;
	}else if(strcmp(splitted, "create") == 0){
		/* Expected format create|name_of_lobby|name_of_player  */
		char *player_name;
		int val;
		splitted = strtok(NULL, DELIMITER);
		
		if(!splitted || !strlen(splitted) || strlen(splitted) > LOBBY_NAME){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Invalid name for lobby !\n", ERR_LOBBY_NAME);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}

		player_name = strtok(NULL, DELIMITER);
		if(!player_name || !strlen(player_name)){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Invalid name for a player !\n", ERR_PLAYER_NOT_FOUND);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}
		printf("Finding player!\n");
		if(find_player(player_name) > 0){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Invalid name for a player !\n", ERR_PLAYER_NOT_FOUND);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}
		printf("Found player!\n");

		if((val = create_lobby(splitted, *p)) < 0){
			clear_buffer(&errbuf);
			if(val == ERR_LOBBY_EXISTS){
				sprintf(errbuf, "%d|Error! Lobby exists !\n", ERR_LOBBY_EXISTS);
			}else{
				sprintf(errbuf, "%d|Error! Lobby couldn't be created, try it again later !\n", ERR_LOBBY);
			}
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}
		printf("Sending lobby response!\n");
		clear_buffer(&infobuf);
		sprintf(infobuf, "%d|%s|Connected\n\r", LOBBY_CREATED, splitted);
		send(client_sock, &infobuf, sizeof(infobuf), 0);

		return INITIALIZATION_SUCCESSFUL;
	}else if(strcmp(splitted, "join") == 0){
	/* Expected format join|lobby_name|player_name */
			char *player_name;
			int val;
		splitted = strtok(NULL, DELIMITER);
		
		if(!splitted || !strlen(splitted) || strlen(splitted) > LOBBY_NAME){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Invalid name for lobby !\n", ERR_LOBBY_NAME);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}

		player_name = strtok(NULL, DELIMITER);
		if(!player_name || !strlen(player_name)){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Player not found !\n", ERR_PLAYER_NOT_FOUND);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;

		}
		
		if(find_player(player_name) > 0){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Player not found !\n", ERR_PLAYER_NOT_FOUND);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}

		if((val = join_lobby(splitted, *p)) < 0){
			if(val == ERR_PLAYER_ALREADY_IN_LOBBY){
				clear_buffer(&errbuf);
				sprintf(errbuf, "%d|Error! Can't join a lobby you're part of !\n", ERR_PLAYER_ALREADY_IN_LOBBY);
				send(client_sock, &errbuf, sizeof(errbuf), 0);
				return INITIALIZATION_UNSUCCESSFUL;
			}
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Lobby couldn't be joined, try it again later !\n", ERR_LOBBY);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}

		clear_buffer(&infobuf);
		sprintf(infobuf, "%d|%s|Joined lobby\n\r", LOBBY_JOINED, splitted);
		send(client_sock, &infobuf, sizeof(infobuf), 0);

		return INITIALIZATION_SUCCESSFUL;
	}else if(strcmp(splitted, "refresh") == 0){
		/* Expected format refresh|player_name */
		char *lobbies;
		splitted = strtok(NULL, DELIMITER);

		if(!splitted || !strlen(splitted)){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Invalid name for name !\n", ERR_PLAYER_NOT_FOUND);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}
		lobbies = get_lobbies();
		clear_buffer(&infobuf);
		sprintf(infobuf, "%d|%s|Sending lobbies!\n\r", LOBBIES_SENT, lobbies);
		send(client_sock, &infobuf, sizeof(infobuf), 0);

		return INITIALIZATION_SUCCESSFUL;

	}else if(strcmp(splitted, "ping") == 0){
		/* Expected format ping|name */
		char *player_name;

		player_name = strtok(NULL, DELIMITER);

		if(!player_name || !strlen(player_name)){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Invalid name for name !\n", ERR_PLAYER_NOT_FOUND);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}

		if(find_player(player_name) > 0){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Player not found !\n", ERR_PLAYER_NOT_FOUND);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}
		if(strcmp(player_name, sock->name)){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Incorrect player name !\n", ERR_INCORRECT_NAME);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}

		sock->pinged = PING_OK;

		return INITIALIZATION_UNSUCCESSFUL;

	}/*else if(strcmp(splitted, "exit") == 0){*/
		/* Expected format exit|player_name|player_id|lobby/null */
		/* if null client wants to disconnect, if not, he just leaves lobby */ 
		/*char *lobby_name, *player_name, *player_id;
		int tmp_pl_id;
		player_name = strtok(NULL, DELIMITER);
		if(!player_name){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Invalid name for disconnect !\n", ERR_LOBBY_NAME);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}
		player_id = strtok(NULL, DELIMITER);
		if(!player_id){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Invalid no player id was found !\n", ERR_LOBBY_NAME);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}

		tmp_pl_id = atoi(player_id);

		if(sock->player_id != tmp_pl_id){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|Error! Invalid player id was found !\n", ERR_LOBBY_NAME);
			send(client_sock, &errbuf, sizeof(errbuf), 0);
			return INITIALIZATION_UNSUCCESSFUL;
		}

		lobby_name = strtok(NULL, DELIMITER);
			clear_buffer(&infobuf);

		if(!lobby_name){
			sprintf(infobuf, "%d|Exiting!\n\r", LOBBIES_SENT);
			send(client_sock, &infobuf, sizeof(infobuf), 0);
			return EXIT;
		}else{
			if(remove_from_lobby(lobby_name, sock) < 0){
				clear_buffer(&errbuf);
				sprintf(errbuf, "%d|Error! Couldn't remove from lobby!\n", ERR_LOBBY_NAME);
				send(client_sock, &errbuf, sizeof(errbuf), 0);
				return INITIALIZATION_UNSUCCESSFUL;
			}else{
				sprintf(infobuf, "%d|%s|Removing from lobby!\n\r", LOBBIES_SENT, lobby_name);
				send(client_sock, &infobuf, sizeof(infobuf), 0);
				return INITIALIZATION_SUCCESSFUL;
			}
			
		}
		return INITIALIZATION_SUCCESSFUL;
	}*/
	return MESSAGE_UNKNOWN;
}

void clear_buffer(char *buffer){
	memset(buffer, ANNULL_VALUE, BUFFER_SIZE);
}

int validate_message(int recv_value, int client_sock){
	char errbuf[BUFFER_SIZE]="ERROR message too long!\n\r";
/*	printf("recv_val %d", recv_value);*/
	if(recv_value == SOCKET_INITIALIZATION_UNSUCCESSESFUL){
			printf("Error!\n\r");
		/*	free(client_sock);*/
		/*	close(client_sock);*/
			return SOCKET_INITIALIZATION_UNSUCCESSESFUL;
	}
	if(recv_value >= BUFFER_SIZE){
		send(client_sock, &errbuf, sizeof(errbuf), 0);
/*		close(client_sock);
		free(arg); */
		return MESSAGE_NOK;	
	} 

	return MESSAGE_OK;
}

int handle_move_message(int client_sock, game *g, player *p, int move_x, int move_y, int other_sock){
		int result, order = p->order, i;
		char *splitted, pl_turn, errbuf[BUFFER_SIZE], infobuf[BUFFER_SIZE];

		printf("x:%d y:%d\n", move_x, move_y);

		result = make_move(g, p, move_x, move_y);

		/* Player attempts an incorrect move */
		if(result == INVALID_MOVE){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|ERROR! Invalid move!\n\r", ERR_INVALID_MOVE);
			send(client_sock, &errbuf, sizeof(errbuf), 0);	
			clear_buffer(&errbuf);
			return MESSAGE_NOK;
		}

		if(result == INCORRECT_TURN_MOVE){
			clear_buffer(&errbuf);
			sprintf(errbuf, "%d|ERROR! Not your turn!\n\r", ERR_WRONG_TURN);
			send(client_sock, &errbuf, sizeof(errbuf), 0);	
			clear_buffer(&errbuf);
			return MESSAGE_NOK;
		}

		/* Player normally put a piece down */
	/*	if(result == MOVE_OK){*/
			clear_buffer(&infobuf);
			sprintf(infobuf, "%d|MOVE_OK\n\r", MOVE_OK);	
			send(client_sock, &infobuf, sizeof(infobuf), 0);
			
			clear_buffer(&infobuf);

			sprintf(infobuf, "%d|%d,%d|Other player\n\r", MOVE_FROM_OTHER_PLAYER, move_x, move_y);
			send(other_sock, &infobuf, sizeof(infobuf), 0);
			clear_buffer(&infobuf);
		/*	return result;
		}*/
		printf("order %d \n", order);
		if(result == PLAYER1_VICTORY){
			if(order == PLAYER1_TURN){

				clear_buffer(&infobuf);
				sprintf(infobuf, "%d|You have won!\n\r", VICTORY);	
				send(other_sock, &infobuf, sizeof(infobuf), 0);
				clear_buffer(&infobuf);

				sprintf(infobuf, "%d|Other player has won!\n\r", DEFEAT);	
				send(client_sock, &infobuf, sizeof(infobuf), 0);
				end_game(g);

				return PLAYER1_TURN;
			}else{
				clear_buffer(&infobuf);
				sprintf(infobuf, "%d|You have won!\n\r", VICTORY);	
				send(client_sock, &infobuf, sizeof(infobuf), 0);
				clear_buffer(&infobuf);

				sprintf(infobuf, "%d|Other player has won!\n\r", DEFEAT);	
				send(other_sock, &infobuf, sizeof(infobuf), 0);
				end_game(g);

				return PLAYER2_TURN;
			}
		}

		if(result == PLAYER2_VICTORY){
			if(order == PLAYER2_TURN){

				clear_buffer(&infobuf);
				sprintf(infobuf, "%d|You have won!\n\r", VICTORY);	
				send(other_sock, &infobuf, sizeof(infobuf), 0);

				clear_buffer(&infobuf);

				sprintf(infobuf, "%d|Other player has won!\n\r", DEFEAT);	
				send(client_sock, &infobuf, sizeof(infobuf), 0);
				end_game(g);
				return PLAYER2_TURN;

			}else{
				clear_buffer(&infobuf);
				sprintf(infobuf, "%d|You have won!\n\r", VICTORY);	
				send(client_sock, &infobuf, sizeof(infobuf), 0);

				clear_buffer(&infobuf);

				sprintf(infobuf, "%d|Other player has won!\n\r", DEFEAT);	
				send(other_sock, &infobuf, sizeof(infobuf), 0);
				end_game(g);
				return PLAYER1_TURN;
			}
			
		}

		/* Last move finished the game in draw */
		if(result == DRAW){
			clear_buffer(&infobuf);
			sprintf(infobuf, "%d|Game ended in draw!\n\r", DRAW);
			send(client_sock, &infobuf, sizeof(infobuf), 0);
			send(other_sock, &infobuf, sizeof(infobuf), 0);
			clear_buffer(&infobuf);
			return DRAW;
		}

		if(result == MOVE_OK){
			return MOVE_OK;
		}
	clear_buffer(&infobuf);
	sprintf(infobuf, "%d|Message unknown!\n\r", UNKNOWN_MESSAGE);
	send(client_sock, &infobuf, sizeof(infobuf), 0);
	return MESSAGE_UNKNOWN;
}


int make_move(game *g, player *p, int move_x, int move_y){
	int state, game_state, order, i;
	/* Validations */
	order = p->order;

	printf("Moving<%d,%d> in game %d!\n", move_x, move_y, g->id);
	if((state = validate_move(g, move_x, move_y, order)) < 0){ /* Errors are negative */
		if(state == INVALID_MOVE){
			return INVALID_MOVE;
		}

		if(state == INCORRECT_TURN_MOVE){
			return INCORRECT_TURN_MOVE;
		}
	}

	g->board[move_x][move_y] = order; 
	g->pointers[move_x]++;

	 for(i = 0 ; i < SIZE_OF_BOARD; i++){
          /*  printf("|%d|", g->pointers[i]);*/
    }
	printf("Move made! Checking victory! <%d, %d> => %d,\n",
	 move_x, move_y, order);

	 g->amount_left--;

	state = check_victory(g, move_x, move_y, order);
	if(g->state == GAME_OVER){

		if(state == DRAW){
			printf("Game ended in draw!\n");
			return DRAW;
		}

		if(order == PLAYER1_VICTORY){
			printf("Player 1 won!\n");
			return PLAYER1_VICTORY;
		}

		printf("Player 2 won!\n");
		return PLAYER2_VICTORY;
	}

	if(g->state == PLAYER1_TURN){
		g->state = PLAYER2_TURN;
		printf("It's player 2's turn now!\n");
	}else{
		g->state = PLAYER1_TURN;
		printf("It's player 1's turn now!\n");
	}

	return MOVE_OK;
}
int reconnect_player(int pl_id){
	printf("Reconnecting!\n");
}

int check_victory(game *g, int move_x, int move_y, int order){
	int i, j, state, next_x, next_y, jump_x, jump_y;
	int directions[DIRECTIONS_LEN][DIRECTIONS] = 
	{{-1,-1}, {1, -1}, 
	{-1, 0}, {1, 0},
	 {-1, 1}, {0, -1}, {1, 1}};
	
	if(!g->amount_left){
		g->state = GAME_OVER;
		return DRAW;
	}

	for(i = 0 ; i < DIRECTIONS_LEN ; i++){
		jump_x = directions[i][0];
		jump_y = directions[i][1];
		printf("directions: <%d, %d>\n", jump_x, jump_y);
		for(j = 1 ; j < VICTORY_AMOUNT; j++){ /* 1 = first jump 0 would mean the place circle it was put */
			next_x = move_x + jump_x * j;
			next_y = move_y + jump_y * j;
			printf("checking move <%d, %d> \n", next_x, next_y);
			/* Out of bounds */
			if(next_x >= SIZE_OF_BOARD || next_y >= SIZE_OF_BOARD || next_x < 0 || next_y < 0){	
				break;
			}
			state = g->board[next_x][next_y];
			/* No circle is here yet */
			if(state == NO_CIRCLE){
				break;
			}
			/* Circle from other player */
			if(state != order){
				break;
			}
			printf("Ok!\n");
		}
		printf("%d\n", j);
			/* 4 in a row were found */
			if(j == VICTORY_AMOUNT){
				g->state = GAME_OVER;
				printf("Player with order: %d won\n", order );
				return VICTORY;
			}
	}
	return MOVE_OK;
}

int validate_move(game *g, int move_x, int move_y, int order){
	
	if(!g){
		return INVALID_PARAMETER;
	}

	if(order != g->state){
		printf("Not your turn! %d X %d\n", order, g->state);
		return INCORRECT_TURN_MOVE;
	}

	if(move_y != g->pointers[move_x]){
		printf("Wrong Y coord! <%d, %d> : %d\n", move_x, move_y, g->pointers[move_x]);
		return INVALID_MOVE;
	}

	if(move_x > SIZE_OF_BOARD || move_y > SIZE_OF_BOARD 
	|| move_x < 0 || move_y < 0 ){
		return INVALID_MOVE;
	}

	return MOVE_OK;

}