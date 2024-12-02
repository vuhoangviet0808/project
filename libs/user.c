#include "user.h"
#include <string.h>
#include <stdio.h>



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

    // Thêm yêu cầu vào danh sách
    if (receiver->request_count < MAX_REQUESTS) {
        receiver->add_friend_requests[receiver->request_count] = sender->id;
        receiver->request_count++;
        return 1;
    }
    return 0; // Danh sách yêu cầu kết bạn đã đầy
}

// Chấp nhận yêu cầu kết bạn
int accept_friend_request(Client *sender, Client *receiver) {
    if(sender->friend_count < MAX_FRIENDS && receiver->friend_count < MAX_FRIENDS){
        sender->friends[sender->friend_count++] = receiver->id;
        receiver->friends[receiver->friend_count++] = receiver->id;
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

// Hủy yêu cầu kết bạn
int cancel_friend_request(Client *sender, Client *receiver) {
    for (int i = 0; i < receiver->request_count; i++) {
        if (receiver->add_friend_requests[i] == sender->id) {
            // Xóa yêu cầu kết bạn khỏi danh sách
            for (int j = i; j < receiver->request_count - 1; j++) {
                receiver->add_friend_requests[j] = receiver->add_friend_requests[j+1];
            }
            receiver->request_count--;
            return 1;
        }
    }
    return 0; // Không tìm thấy yêu cầu kết bạn để hủy
}

// Lấy danh sách bạn bè
char* get_friends(Client user) {
    char listfr[BUFFER_SIZE];
    listfr[0] = '\0';
    for(int i=0;i<user.friend_count;i++){
        char temp[10];
        sprintf(temp, "%d", user.friends[i]);
        strcat(listfr, temp);
        if(i < user.friend_count-1){
            strcat(listfr, " ");
        }
    }
    strcat(listfr, '\0');
    return listfr;
}

// Lấy danh sách yêu cầu kết bạn
void get_friend_requests(Client *user, int request_list[], int *request_count) {
    *request_count = user->request_count;
    for (int i = 0; i < user->request_count; i++) {
        request_list[i] = user->add_friend_requests[i];
    }
}