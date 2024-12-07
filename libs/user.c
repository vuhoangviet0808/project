#include "user.h"
#include <string.h>
#include <stdio.h>
#include<stdlib.h>


int is_number(const char *str){
    if(str == NULL || *str == '\0'){
        return 0;
    }
    char *endptr;
    strtol(str, &endptr, 10);
    return *endptr == '\0';
}
// Gửi yêu cầu kết bạn
int send_friend_request(Client *sender, Client *receiver) {
    for (int i = 0; i < receiver->request_count; i++) {
        if (receiver->add_friend_requests[i] == sender->id) {
            return 0; 
        }
    }
    // for (int i = 0; i < receiver->friend_count; i++) {
    //     if (receiver->friends[i] == sender->id) {
    //         return 0; 
    //     }
    // }

    // Thêm yêu cầu vào danh sách
    if (receiver->request_count < MAX_REQUESTS) {
        receiver->add_friend_requests[receiver->request_count] = sender->id;
        receiver->request_count++;
        return 1;
    }
    return 0; // Danh sách yêu cầu kết bạn đã đầy
}

int accept_friend_request(Client *sender, Client *receiver) {
    if(sender->friend_count < MAX_FRIENDS && receiver->friend_count < MAX_FRIENDS){
        sender->friends[sender->friend_count++] = receiver->id;
        receiver->friends[receiver->friend_count++] = sender->id;
        return 1;
    }
    return 0;
}

// Từ chối yêu cầu kết bạn
int decline_friend_request(Client *user, int sender_id) {
    for (int i = 0; i < user->request_count; i++) {
        if (user->add_friend_requests[i] == sender_id) {
            // Xóa yêu cầu kết bạn khỏi danh sách
            for (int j = i; j < user->request_count - 1; j++) {
                user->add_friend_requests[j] = user->add_friend_requests[j+1];
            }
            user->request_count--;
            return 1;
        }
    }
    return 0; // Không tìm thấy yêu cầu kết bạn
}

int cancel_friend_request(Client *sender, Client *receiver) {
    for (int i = 0; i < receiver->request_count; i++) {
        if (receiver->add_friend_requests[i] == sender->id) {
            for (int j = i; j < receiver->request_count - 1; j++) {
                receiver->add_friend_requests[j] = receiver->add_friend_requests[j+1];
            }
            receiver->request_count--;
            return 1;
        }
    }
    return 0; 
}
char* get_friends(Client user) {
    size_t buffer_size = user.friend_count * 4; 
    char* listfr = (char*)malloc(buffer_size);
    if (!listfr) {
        perror("malloc failed");
        return NULL;
    }
    listfr[0] = '\0'; 

    for (int i = 0; i < user.friend_count; i++) {
        char temp[12];
        snprintf(temp, sizeof(temp), "%d", user.friends[i]); 
        strcat(listfr, temp);
        if (i < user.friend_count - 1) {
            strcat(listfr, " "); 
        }
    }
    return listfr; 
}


// Lấy danh sách yêu cầu kết bạn
void get_friend_requests(Client *user, int request_list[], int *request_count) {
    *request_count = user->request_count;
    for (int i = 0; i < user->request_count; i++) {
        request_list[i] = user->add_friend_requests[i];
    }
}


int remove_friend(Client* sender, Client* receiver){
    int friend_id = receiver->id;
    for (int i = 0; i < sender->friend_count; i++) {
        if (sender->friends[i] == friend_id) { 
            for (int j = i; j < sender->friend_count - 1; j++) {
                sender->friends[j] = sender->friends[j + 1];
            }
            sender->friend_count--;
            break; 
        }
    }
    friend_id = sender->id;
    for (int i = 0; i < receiver->friend_count; i++) {
        if (receiver->friends[i] == friend_id) { 
            for (int j = i; j < receiver->friend_count - 1; j++) {
                receiver->friends[j] = receiver->friends[j + 1];
            }
            receiver->friend_count--;
            return 1; 
        }
    }
    return 0; 
}
