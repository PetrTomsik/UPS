#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "player.h"

int handle_message(int client_sock, char *message, player **p, int other_sock);
int validate_message(int recv_value, int client_sock);
int check_player_turn(int pl_turn);
int make_move(game *g, player *p, int move_x, int move_y);
int reconnect_player(int pl_id);
void clear_buffer(char *buffer);
int check_victory(game *g, int move_x, int move_y, int order);
int validate_move(game *g, int move_x, int move_y, int order);
int handle_move_message(int client_sock, game *g, player *p, int move_x, int move_y, int other_sock);
#endif