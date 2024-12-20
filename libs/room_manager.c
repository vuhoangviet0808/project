#include "room_manager.h"
#include "common.h"
#include "utils.h"
// Định nghĩa biến toàn cục
ChatRoom rooms[MAX_ROOMS];
pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;
extern Client clients[MAX_CLIENTS];
// Khởi tạo danh sách phòng

int next_room_id = 0;

void load_next_room_id()
{
    FILE *file = fopen("../server/room_data/next_id_room.txt", "r");
    if (file)
    {
        fscanf(file, "%d", &next_room_id);
        fclose(file);
    }
    else
    {
        next_room_id = 0; // Nếu file không tồn tại
    }
}
void load_rooms()
{
    pthread_mutex_lock(&rooms_mutex);

    FILE *file = fopen("../server/room_data/room_list.txt", "r");
    if (file)
    {
        char line[BUFFER_SIZE];
        while (fgets(line, sizeof(line), file))
        {
            int room_id;
            char room_name[BUFFER_SIZE];
            int creator_id;
            if (sscanf(line, "%d %s %d", &room_id, room_name, &creator_id) == 3)
            {
                // Khôi phục thông tin phòng
                rooms[room_id].id = room_id;
                strncpy(rooms[room_id].name, room_name, BUFFER_SIZE);
                rooms[room_id].name[BUFFER_SIZE - 1] = '\0';
                rooms[room_id].member_count = 0;
                rooms[room_id].creator_id = creator_id;
                // Đọc danh sách thành viên từ file room_<name>.txt
                char room_file[BUFFER_SIZE];
                snprintf(room_file, sizeof(room_file), "../server/room_data/room_%s.txt", room_name);
                FILE *room_file_ptr = fopen(room_file, "r");
                if (room_file_ptr)
                {
                    char member_line[BUFFER_SIZE];
                    while (fgets(member_line, sizeof(member_line), room_file_ptr))
                    {
                        char command[BUFFER_SIZE], username[BUFFER_SIZE];
                        int user_id;

                        // Xử lý lệnh JOIN
                        if (sscanf(member_line, "%s %s %d", command, username, &user_id) == 3 &&
                            strcmp(command, "JOIN") == 0)
                        {
                            int already_added = 0;
                            for (int i = 0; i < rooms[room_id].member_count; i++)
                            {
                                if (rooms[room_id].members[i] == user_id)
                                {
                                    already_added = 1;
                                    break;
                                }
                            }

                            if (!already_added && rooms[room_id].member_count < MAX_CLIENTS)
                            {
                                rooms[room_id].members[rooms[room_id].member_count++] = user_id;
                            }
                        }
                        // Xử lý lệnh ADD
                        else if (sscanf(member_line, "%s %s %d", command, username, &user_id) == 3 &&
                                 strcmp(command, "ADD") == 0)
                        {
                            int already_added = 0;
                            for (int i = 0; i < rooms[room_id].member_count; i++)
                            {
                                if (rooms[room_id].members[i] == user_id)
                                {
                                    already_added = 1;
                                    break;
                                }
                            }

                            if (!already_added && rooms[room_id].member_count < MAX_CLIENTS)
                            {
                                rooms[room_id].members[rooms[room_id].member_count++] = user_id;
                            }
                        }
                        // Xử lý lệnh LEAVE
                        else if (sscanf(member_line, "%s %s %d", command, username, &user_id) == 3 &&
                                 strcmp(command, "LEAVE") == 0)
                        {
                            for (int i = 0; i < rooms[room_id].member_count; i++)
                            {
                                if (rooms[room_id].members[i] == user_id)
                                {
                                    // Dịch chuyển danh sách để loại bỏ user
                                    for (int j = i; j < rooms[room_id].member_count - 1; j++)
                                    {
                                        rooms[room_id].members[j] = rooms[room_id].members[j + 1];
                                    }
                                    rooms[room_id].member_count--;
                                    break;
                                }
                            }
                        }
                        // Xử lý lệnh REMOVE
                        else if (sscanf(member_line, "%s %s %d", command, username, &user_id) == 3 &&
                                 strcmp(command, "REMOVE") == 0)
                        {
                            for (int i = 0; i < rooms[room_id].member_count; i++)
                            {
                                if (rooms[room_id].members[i] == user_id)
                                {
                                    // Dịch chuyển danh sách để loại bỏ user
                                    for (int j = i; j < rooms[room_id].member_count - 1; j++)
                                    {
                                        rooms[room_id].members[j] = rooms[room_id].members[j + 1];
                                    }
                                    rooms[room_id].member_count--;
                                    break;
                                }
                            }
                        }
                    }
                    fclose(room_file_ptr);
                }
                else
                {
                    printf("Room file not found for room: %s\n", room_name);
                }
            }
        }
        fclose(file);
    }
    else
    {
        perror("Failed to open room_list.txt");
    }

    pthread_mutex_unlock(&rooms_mutex);
}

void save_next_room_id()
{
    FILE *file = fopen("../server/room_data/next_id_room.txt", "w");
    if (file)
    {
        fprintf(file, "%d", next_room_id);
        fclose(file);
    }
}

void init_rooms()
{
    pthread_mutex_init(&rooms_mutex, NULL);
    for (int i = 0; i < MAX_ROOMS; i++)
    {
        rooms[i].id = -1;
        rooms[i].member_count = 0;
    }
}

int create_room(const char *room_name, int creator_id)
{
    pthread_mutex_lock(&rooms_mutex);

    if (next_room_id < MAX_ROOMS)
    {
        int room_id = next_room_id++;
        save_next_room_id();

        rooms[room_id].id = room_id;
        strncpy(rooms[room_id].name, room_name, BUFFER_SIZE);
        rooms[room_id].creator_id = creator_id; // Gán ID người tạo
        rooms[room_id].member_count = 0;

        // Ghi thông tin phòng vào room_list.txt
        FILE *file = fopen("../server/room_data/room_list.txt", "a");
        if (file)
        {
            fprintf(file, "%d %s %d\n", room_id, room_name, creator_id);
            fclose(file);
        }
        else
        {
            perror("Failed to open room_list.txt");
        }

        // Tạo file room_<name>.txt
        char room_file[BUFFER_SIZE];
        snprintf(room_file, sizeof(room_file), "../server/room_data/room_%s.txt", room_name);
        FILE *room_fp = fopen(room_file, "w");
        if (room_fp)
        {
            fclose(room_fp); // Chỉ tạo file rỗng
        }
        else
        {
            perror("Failed to create room file");
        }

        pthread_mutex_unlock(&rooms_mutex);
        return room_id;
    }

    pthread_mutex_unlock(&rooms_mutex);
    return -1; // Không thể tạo thêm phòng
}

// Tham gia phòng chat
int join_room(int room_id, int user_id)
{
    pthread_mutex_lock(&rooms_mutex);

    // Kiểm tra nếu phòng tồn tại
    if (room_id < 0 || room_id >= MAX_ROOMS || rooms[room_id].id == -1)
    {
        pthread_mutex_unlock(&rooms_mutex);
        printf("Room with ID %d does not exist.\n", room_id);
        return 0; // Phòng chưa được tạo
    }

    // Kiểm tra nếu người dùng đã tham gia
    for (int i = 0; i < rooms[room_id].member_count; i++)
    {
        if (rooms[room_id].members[i] == user_id)
        {
            pthread_mutex_unlock(&rooms_mutex);
            printf("User with ID %d already joined room %d.\n", user_id, room_id);
            return 0; // Đã tham gia
        }
    }

    // Thêm người dùng vào phòng
    if (rooms[room_id].member_count < MAX_CLIENTS)
    {
        rooms[room_id].members[rooms[room_id].member_count++] = user_id;

        // Ghi vào file room_<name>.txt
        char room_file[BUFFER_SIZE];
        snprintf(room_file, sizeof(room_file), "../server/room_data/room_%s.txt", rooms[room_id].name);
        FILE *file = fopen(room_file, "a");
        if (file)
        {
            fprintf(file, "JOIN %s %d\n", clients[user_id].username, user_id); // Ghi tên và ID
            fclose(file);
        }
        else
        {
            perror("Failed to open room file");
            pthread_mutex_unlock(&rooms_mutex);
            return 0; // Lỗi ghi file
        }

        pthread_mutex_unlock(&rooms_mutex);
        printf("User with ID %d successfully joined room %d.\n", user_id, room_id);
        return 1; // Tham gia thành công
    }
    else
    {
        printf("Room %d is full.\n", room_id);
    }

    pthread_mutex_unlock(&rooms_mutex);
    return 0; // Tham gia thất bại
}

int add_user_to_room(int room_id, int user_id)
{
    pthread_mutex_lock(&rooms_mutex);

    // Kiểm tra phòng có tồn tại
    if (room_id < 0 || room_id >= MAX_ROOMS || rooms[room_id].id == -1)
    {
        pthread_mutex_unlock(&rooms_mutex);
        return 0; // Phòng không tồn tại
    }

    // Kiểm tra nếu người dùng đã có trong nhóm
    for (int i = 0; i < rooms[room_id].member_count; i++)
    {
        if (rooms[room_id].members[i] == user_id)
        {
            pthread_mutex_unlock(&rooms_mutex);
            return 0; // Người dùng đã có trong nhóm
        }
    }

    // Thêm người dùng vào nhóm
    if (rooms[room_id].member_count < MAX_CLIENTS)
    {
        rooms[room_id].members[rooms[room_id].member_count++] = user_id;

        // Ghi vào file room_<name>.txt
        char room_file[BUFFER_SIZE];
        snprintf(room_file, sizeof(room_file), "../server/room_data/room_%s.txt", rooms[room_id].name);
        FILE *file = fopen(room_file, "a");
        if (file)
        {
            fprintf(file, "ADD %s %d\n", clients[user_id].username, user_id);
            fclose(file);

            // Gửi thông báo đến các thành viên khác trong phòng
            char notify_message[BUFFER_SIZE];
            snprintf(notify_message, sizeof(notify_message), "User %s (ID: %d) was added to room %s.\n",
                     clients[user_id].username, user_id, rooms[room_id].name);

            for (int i = 0; i < rooms[room_id].member_count; i++)
            {
                int member_id = rooms[room_id].members[i];
                if (clients[member_id].is_online)
                {
                    send(clients[member_id].socket, notify_message, strlen(notify_message), 0);
                }
            }
        }
        else
        {
            perror("Failed to open room file");
            pthread_mutex_unlock(&rooms_mutex);
            return 0; // Lỗi ghi file
        }

        pthread_mutex_unlock(&rooms_mutex);
        return 1; // Thêm thành công
    }

    pthread_mutex_unlock(&rooms_mutex);
    return 0; // Phòng đã đầy
}

int leave_room(int room_id, int user_id)
{
    pthread_mutex_lock(&rooms_mutex);

    // Kiểm tra nếu phòng tồn tại
    if (room_id < 0 || room_id >= MAX_ROOMS || rooms[room_id].id == -1)
    {
        pthread_mutex_unlock(&rooms_mutex);
        return 0; // Phòng không tồn tại
    }

    // Xóa thành viên khỏi danh sách
    int member_removed = 0;
    for (int i = 0; i < rooms[room_id].member_count; i++)
    {
        if (rooms[room_id].members[i] == user_id)
        {
            // Dịch các thành viên phía sau lên
            for (int j = i; j < rooms[room_id].member_count - 1; j++)
            {
                rooms[room_id].members[j] = rooms[room_id].members[j + 1];
            }
            rooms[room_id].member_count--;
            member_removed = 1;
            break;
        }
    }

    if (!member_removed)
    {
        pthread_mutex_unlock(&rooms_mutex);
        return 0; // Người dùng không nằm trong phòng
    }

    // Ghi vào file room_<name>.txt
    char room_file[BUFFER_SIZE];
    snprintf(room_file, sizeof(room_file), "../server/room_data/room_%s.txt", rooms[room_id].name);
    FILE *file = fopen(room_file, "a");
    if (file)
    {
        fprintf(file, "LEAVE %s %d\n", clients[user_id].username, user_id);
        fclose(file);
    }
    else
    {
        perror("Failed to open room file");
    }

    // Gửi thông báo tới các thành viên khác trong phòng
    for (int i = 0; i < rooms[room_id].member_count; i++)
    {
        int member_id = rooms[room_id].members[i];
        // if (clients[member_id].is_online)
        // {
        char notification[BUFFER_SIZE];
        snprintf(notification, sizeof(notification), "User %s has leave the room %s.\n",
                 clients[user_id].username, rooms[room_id].name);
        send(clients[member_id].socket, notification, strlen(notification), 0);
        // }
    }
    // Thông báo trên server
    printf("User %s (ID: %d) has leave room %s (ID: %d).\n",
           clients[user_id].username, user_id, rooms[room_id].name, room_id);

    pthread_mutex_unlock(&rooms_mutex);
    return 1; // Rời phòng thành công
}

int remove_user_from_room(int room_id, int remover_id, int user_id_to_remove)
{
    pthread_mutex_lock(&rooms_mutex);

    // Kiểm tra nếu phòng tồn tại
    if (room_id < 0 || room_id >= MAX_ROOMS || rooms[room_id].id == -1)
    {
        pthread_mutex_unlock(&rooms_mutex);
        return 0;
    }

    // Kiểm tra nếu remover là người tạo phòng
    if (rooms[room_id].creator_id != remover_id)
    {
        // Gửi thông báo lỗi tới client
        char error_message[BUFFER_SIZE];
        snprintf(error_message, sizeof(error_message), "error_not_creator You are not the creator of the room and cannot remove members.");
        send_websocket_message(clients[remover_id].socket, error_message, strlen(error_message), 0);

        pthread_mutex_unlock(&rooms_mutex);
        return 0;
    }

    // Tìm và xóa user khỏi danh sách
    int user_found = 0;
    for (int i = 0; i < rooms[room_id].member_count; i++)
    {
        if (rooms[room_id].members[i] == user_id_to_remove)
        {
            for (int j = i; j < rooms[room_id].member_count - 1; j++)
            {
                rooms[room_id].members[j] = rooms[room_id].members[j + 1];
            }
            rooms[room_id].member_count--;
            user_found = 1;
            break;
        }
    }

    if (!user_found)
    {
        pthread_mutex_unlock(&rooms_mutex);
        return 0;
    }

    // Ghi vào file room_<name>.txt
    char room_file[BUFFER_SIZE];
    snprintf(room_file, sizeof(room_file), "../server/room_data/room_%s.txt", rooms[room_id].name);
    FILE *file = fopen(room_file, "a");
    if (file)
    {
        fprintf(file, "REMOVE %s %d\n", clients[user_id_to_remove].username, user_id_to_remove);
        fclose(file);
    }
    else
    {
        perror("Failed to open room file");
    }

    // Gửi thông báo tới người bị xóa
    char notification[BUFFER_SIZE];
    snprintf(notification, sizeof(notification), "You have been removed from the room %s by the creator.", rooms[room_id].name);
    send_websocket_message(clients[user_id_to_remove].socket, notification, strlen(notification), 0);

    pthread_mutex_unlock(&rooms_mutex);
    return 1;
}

// int room_message(int room_id, int sender_id, const char *message)
// {

//     pthread_mutex_lock(&rooms_mutex);

//     if (room_id >= 0 && room_id < MAX_ROOMS && rooms[room_id].id != -1)
//     {
//         // Lưu tin nhắn vào file room_<name>.txt
//         char room_file[BUFFER_SIZE];
//         snprintf(room_file, sizeof(room_file), "../server/room_data/room_%s.txt", rooms[room_id].name);
//         FILE *file = fopen(room_file, "a");
//         if (file)
//         {
//             fprintf(file, "MESSAGE %s %d %s\n", clients[sender_id].username, sender_id, message);
//             fclose(file);
//         }
//         else
//         {
//             perror("Failed to open room file");
//             pthread_mutex_unlock(&rooms_mutex);
//             return 0; // Lỗi ghi file
//         }

//         // Gửi tin nhắn đến các thành viên trong phòng
//         for (int i = 0; i < rooms[room_id].member_count; i++)
//         {
//             int member_id = rooms[room_id].members[i];
//             // if (clients[member_id].is_online)
//             // {
//             char full_message[BUFFER_SIZE];
//             snprintf(full_message, sizeof(full_message), "Room %d [%s]: %s\n",
//                      room_id, clients[sender_id].username, message);
//             send(clients[member_id].socket, full_message, strlen(full_message), 0);
//             // }
//         }

//         pthread_mutex_unlock(&rooms_mutex);
//         return 1; // Gửi thành công
//     }

//     pthread_mutex_unlock(&rooms_mutex);
//     return 0; // Gửi thất bại
// }
int room_message(int room_id, int sender_id, const char *message, int client_sock)
{
    pthread_mutex_lock(&rooms_mutex);

    if (room_id >= 0 && room_id < MAX_ROOMS && rooms[room_id].id != -1)
    {
        // Kiểm tra xem sender có phải là thành viên của nhóm không
        int is_member = 0;
        for (int i = 0; i < rooms[room_id].member_count; i++)
        {
            if (rooms[room_id].members[i] == sender_id)
            {
                is_member = 1;
                break;
            }
        }

        if (!is_member)
        {
            pthread_mutex_unlock(&rooms_mutex);
            printf("User %d is not a member of room %d.\n", sender_id, room_id);
            send_websocket_message(client_sock, "error_not_a_member", strlen("error_not_a_member"), 0);
            return 0; // Người gửi không phải là thành viên
        }

        // Lưu tin nhắn vào file
        char room_file[BUFFER_SIZE];
        snprintf(room_file, sizeof(room_file), "../server/room_data/room_%s.txt", rooms[room_id].name);

        FILE *file = fopen(room_file, "a");
        if (file)
        {
            fprintf(file, "MESSAGE %s %s\n", clients[sender_id].username, message);
            fclose(file);
        }
        else
        {
            perror("Failed to open room file");
            pthread_mutex_unlock(&rooms_mutex);
            return 0;
        }

        // Gửi tin nhắn đến tất cả thành viên
        for (int i = 0; i < rooms[room_id].member_count; i++)
        {
            int member_id = rooms[room_id].members[i];
            char full_message[BUFFER_SIZE];
            snprintf(full_message, sizeof(full_message), "new_room_message %s %s", clients[sender_id].username, message);
            send_websocket_message(clients[member_id].socket, full_message, strlen(full_message), 0);
        }

        pthread_mutex_unlock(&rooms_mutex);
        return 1;
    }

    pthread_mutex_unlock(&rooms_mutex);
    return 0;
}

void get_user_rooms(int user_id, char *result)
{

    pthread_mutex_lock(&rooms_mutex);
    result[0] = '\0'; // Xóa chuỗi trước khi thêm dữ liệu mới

    for (int i = 0; i < MAX_ROOMS; i++)
    {
        if (rooms[i].id != -1) // Nếu phòng tồn tại
        {
            for (int j = 0; j < rooms[i].member_count; j++)
            {
                if (rooms[i].members[j] == user_id)
                {
                    // Thêm tên phòng vào kết quả (chỉ tên phòng)
                    char room_info[BUFFER_SIZE];
                    snprintf(room_info, sizeof(room_info), "%s:%d\n", rooms[i].name, rooms[i].id);
                    strncat(result, room_info, BUFFER_SIZE - strlen(result) - 1);
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&rooms_mutex);
}
void get_room_members(int room_id, char *result)
{
    pthread_mutex_lock(&rooms_mutex);

    if (room_id >= 0 && room_id < MAX_ROOMS && rooms[room_id].id != -1)
    {
        snprintf(result, BUFFER_SIZE, "members_list "); // Thêm tiền tố `members_list`
        for (int i = 0; i < rooms[room_id].member_count; i++)
        {
            char member_info[BUFFER_SIZE];
            snprintf(member_info, sizeof(member_info), "%s (ID: %d)\n",
                     clients[rooms[room_id].members[i]].username,
                     clients[rooms[room_id].members[i]].id); // Bao gồm cả username và ID
            strncat(result, member_info, BUFFER_SIZE - strlen(result) - 1);
        }
    }
    else
    {
        snprintf(result, BUFFER_SIZE, "members_list Room not found or invalid room ID.");
    }

    pthread_mutex_unlock(&rooms_mutex);
}
