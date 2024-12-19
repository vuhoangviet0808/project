#ifndef ROOM_MANAGER_H
#define ROOM_MANAGER_H

#include "common.h"

// Khai báo các hàm quản lý phòng chat
// void init_rooms();
void init_rooms();
void load_next_room_id();
void load_rooms();
void save_next_room_id();
int create_room(const char *room_name, int creator_id);
int join_room(int room_id, int user_id);
int room_message(int room_id, int user_id, const char *message, int client_sock);
int add_user_to_room(int room_id, int user_id);
int leave_room(int room_id, int user_id);
int remove_user_from_room(int room_id, int remover_id, int user_id_to_remove);
void get_user_rooms(int user_id, char *result);
void get_room_members(int room_id, char *result);

#endif