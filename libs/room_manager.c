#include "room_manager.h"
#include "common.h"
// Định nghĩa biến toàn cục
ChatRoom rooms[MAX_ROOMS];
pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;

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

            if (sscanf(line, "%d %s", &room_id, room_name) == 2)
            {
                // Khôi phục thông tin phòng
                rooms[room_id].id = room_id;
                strncpy(rooms[room_id].name, room_name, BUFFER_SIZE);
                rooms[room_id].name[BUFFER_SIZE - 1] = '\0';
                rooms[room_id].member_count = 0;

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

                        // Khôi phục thông tin JOIN
                        if (sscanf(member_line, "%s %s %d", command, username, &user_id) == 3 &&
                            strcmp(command, "JOIN") == 0)
                        {
                            // Kiểm tra và thêm vào danh sách thành viên
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
        printf("Room with ID %d does not exist.\n", room_id);
        return 0; // Phòng không tồn tại
    }

    // Kiểm tra nếu người thực hiện là người tạo nhóm
    if (rooms[room_id].creator_id != remover_id)
    {
        pthread_mutex_unlock(&rooms_mutex);
        printf("User %d is not the creator of room %d. Cannot remove members.\n", remover_id, room_id);
        return 0; // Không có quyền xóa
    }

    // Kiểm tra nếu người bị xóa nằm trong danh sách thành viên
    int user_found = 0;
    for (int i = 0; i < rooms[room_id].member_count; i++)
    {
        if (rooms[room_id].members[i] == user_id_to_remove)
        {
            // Dịch các thành viên phía sau lên
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
        printf("User with ID %d is not in room %d.\n", user_id_to_remove, room_id);
        return 0; // Người dùng không nằm trong phòng
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

    // Gửi thông báo tới các thành viên khác trong phòng
    for (int i = 0; i < rooms[room_id].member_count; i++)
    {
        int member_id = rooms[room_id].members[i];
        // if (clients[member_id].is_online)
        // {
        char notification[BUFFER_SIZE];
        snprintf(notification, sizeof(notification), "User %s has been removed from room %s by the creator.\n",
                 clients[user_id_to_remove].username, rooms[room_id].name);
        send(clients[member_id].socket, notification, strlen(notification), 0);
        // }
    }

    // Thông báo trên server
    printf("User %s (ID: %d) has been removed from room %s (ID: %d) by the creator (ID: %d).\n",
           clients[user_id_to_remove].username, user_id_to_remove,
           rooms[room_id].name, room_id, remover_id);

    pthread_mutex_unlock(&rooms_mutex);
    return 1; // Xóa thành công
}

int room_message(int room_id, int sender_id, const char *message)
{

    pthread_mutex_lock(&rooms_mutex);

    if (room_id >= 0 && room_id < MAX_ROOMS && rooms[room_id].id != -1)
    {
        // Lưu tin nhắn vào file room_<name>.txt
        char room_file[BUFFER_SIZE];
        snprintf(room_file, sizeof(room_file), "../server/room_data/room_%s.txt", rooms[room_id].name);
        FILE *file = fopen(room_file, "a");
        if (file)
        {
            fprintf(file, "MESSAGE %s %d %s\n", clients[sender_id].username, sender_id, message);
            fclose(file);
        }
        else
        {
            perror("Failed to open room file");
            pthread_mutex_unlock(&rooms_mutex);
            return 0; // Lỗi ghi file
        }

        // Gửi tin nhắn đến các thành viên trong phòng
        for (int i = 0; i < rooms[room_id].member_count; i++)
        {
            int member_id = rooms[room_id].members[i];
            // if (clients[member_id].is_online)
            // {
            char full_message[BUFFER_SIZE];
            snprintf(full_message, sizeof(full_message), "Room %d [%s]: %s\n",
                     room_id, clients[sender_id].username, message);
            send(clients[member_id].socket, full_message, strlen(full_message), 0);
            // }
        }

        pthread_mutex_unlock(&rooms_mutex);
        return 1; // Gửi thành công
    }

    pthread_mutex_unlock(&rooms_mutex);
    return 0; // Gửi thất bại
}
void get_user_rooms(int user_id, char *result)
{
    pthread_mutex_lock(&rooms_mutex);
    result[0] = '\0'; // Xóa chuỗi trước khi thêm dữ liệu mới

    for (int i = 0; i < MAX_ROOMS; i++)
    {
        if (rooms[i].id != -1)
        { // Nếu phòng tồn tại
            for (int j = 0; j < rooms[i].member_count; j++)
            {
                if (rooms[i].members[j] == user_id)
                {
                    // Thêm tên phòng vào kết quả
                    char room_info[BUFFER_SIZE];
                    snprintf(room_info, sizeof(room_info), "Room ID: %d, Name: %s\n", rooms[i].id, rooms[i].name);
                    strncat(result, room_info, BUFFER_SIZE - strlen(result) - 1);
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&rooms_mutex);
}
