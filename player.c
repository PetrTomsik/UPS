#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "player.h"
#include "message_handler.h"
#include "player_db.h"


int player_counter = 0;

void *serve_player(void *arg){
	int client_sock, recv_value, message_value, order, other_sock;
	char cbuf[BUFFER_SIZE], buf[BUFFER_SIZE], errbuf[BUFFER_SIZE];
	player *p;
	game *g;
	
	client_sock = (int) arg;
	
	printf("\n\nNew thread! %d\n\n", arg);

	strcpy(errbuf, "ERROR\n\r");
	
/*	sock = (int *) arg;
	client_sock = sock->socket_num;*/
	recv_value = recv(client_sock, &cbuf, sizeof(char) * BUFFER_SIZE, 0);

	while((message_value = validate_message(recv_value, client_sock)) != MESSAGE_OK){
		if(message_value == SOCKET_INITIALIZATION_UNSUCCESSESFUL){
			printf("Error with client!\n");
			close(client_sock);
			free(arg);
			return;
		}
		if(message_value == MESSAGE_NOK){

		}
		memset(&cbuf, 0, BUFFER_SIZE);

		recv_value = recv(client_sock, &cbuf, sizeof(char) * BUFFER_SIZE, 0);

	}
	printf("First Message: %s!\n", cbuf);
	message_value = handle_message(client_sock, cbuf, &p, -1);

	if(message_value != INITIALIZATION_SUCCESSFUL){
		printf("Initialization failed!\n");
		close(client_sock);
		return;
	}



	while(1){
		other_sock = p->other_sock;
		printf("clearing !\n");
		clear_buffer(&cbuf);

		recv_value = recv(client_sock, &cbuf, sizeof(char) * BUFFER_SIZE, 0);

		if(recv_value == EXIT){ /* DC */
			break;
		}
		if(validate_message(recv_value, client_sock) == SOCKET_INITIALIZATION_UNSUCCESSESFUL){	
			break;
		}		
			
		
		printf("\n\nMessage: %s r:%d m:%d !\n\n", cbuf, recv_value, message_value);
		message_value = handle_message(client_sock, cbuf, &p, other_sock);

	

	/*	if(message_value == PLAYER1_VICTORY){
			printf("Player1 might've won! %d : victory: %d\n", sock->p->order, PLAYER1_TURN);
			if((order = sock->p->order) == PLAYER1_TURN){
				memset(&buf, 0, sizeof(buf));
				sprintf(buf, "%d%s", PLAYER1_VICTORY,"|You have won!\n\r");
				send(client_sock, &buf, sizeof(buf), 0);
				printf("Player 1 has won !\n", sizeof(buf), sizeof(*buf));
			}else{
				memset(&buf, 0, sizeof(buf));
				sprintf(buf, "%d%s", PLAYER2_VICTORY,"|You have lost!\n\r");
				send(client_sock, &buf, sizeof(buf), 0);
			}
			break;
		}

		if(message_value == PLAYER2_VICTORY){
			printf("Player2 might've won! %d\n", sock->p->order);
			if((order = sock->p->order) == PLAYER2_TURN){
				memset(&buf, 0, sizeof(buf));
				sprintf(buf, "%d%s", PLAYER2_VICTORY,"|You have won!\n\r");
				send(client_sock, &buf, sizeof(buf), 0);
				printf("Player 2 has won !\n", sizeof(buf), sizeof(*buf));
			}else{
				memset(&buf, 0, sizeof(buf));
				sprintf(buf, "%d%s", PLAYER2_VICTORY,"|You have lost!\n\r");
				send(client_sock, &buf, sizeof(buf), 0);
			}
			break;

		}

		if(message_value == DRAW){
				memset(&buf, 0, sizeof(buf));
				sprintf(buf, "%d%s", DRAW,"|Game ended in a draw!\n\r");
				send(client_sock, &buf, sizeof(buf), 0);
				printf("Draw !\n", sizeof(buf), sizeof(*buf));
				break;
		}*/
	
	

	}

	close(client_sock);

	return 0;
}


int *handle_ping(void *arg){
	player *p;
	int flag;

	p = (player *) arg;
	printf("Handling pings!\n");
	if(!p){
		printf("Error player not sent to pinger!\n");
		return INITIALIZATION_UNSUCCESSFUL;
	}

	while(1){
		if(p->pinged == PING_OK){
			printf("Ping arrived from %s !\n", p->name);
			p->pinged = WAITING_PING;
			flag = 0;
		}else{
			if(flag){
				if(p->state == IN_LOBBY){
					remove_from_lobby(p->curr_lobby, p);
					remove_lobby(p->curr_lobby);
					remove_player(p);
					return DISCONNECTED;
				}

				p->state = DISCONNECTED;
				printf("\n\nPlayer disconnected!\n\n");
				sleep(20);	/* Time out for the player to reconnect */
				if(p->state != DISCONNECTED){
					continue;
				}
				
				printf("Player is completelly gone !\n");
				forcefully_end_game(p, p->g);
				remove_player(p);
				return DISCONNECTED;
			}else{
				printf("Setting up flag!\n");
				flag = 1;
			}
		}
		sleep(10);
	}

	return INITIALIZATION_SUCCESSFUL;
}


int forcefully_end_game(player *p, struct GAME *g){
	int sock, other_sock;
	char infbuf[BUFFER_SIZE];
	if(!p || !g){
		printf("Invalid params! - forcefully end game\n");
	}

	sprintf(infbuf, "%d|Other player  has disconnected! You have won!\n\r", VICTORY);
	send(p->other_sock, &infbuf, sizeof(infbuf), 0);

	end_game(g);


}
