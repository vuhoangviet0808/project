#include "user_manager.h"

int load_next_id()
{
    int next_id;
    FILE *file = fopen(ID_FILE, "r");
    if (file)
    {
        fscanf(file, "%d", &next_id);
        fclose(file);
    }
    else
    {
        next_id = 0;
    }
    return next_id;
}

void save_next_id(int next_id)
{
    FILE *file = fopen(ID_FILE, "w");
    if (file)
    {
        fprintf(file, "%d", next_id);
        fclose(file);
    }
}

int create_user_directory(const char *username, const char *password)
{
    char user_dir[BUFFER_SIZE];
    snprintf(user_dir, sizeof(user_dir), "%s/%s", BASE_DIR, username);

    if (mkdir(user_dir, 0700) == 0)
    {
        int user_id = load_next_id();
        int next_id = user_id + 1;
        save_next_id(next_id);

        char info_file[BUFFER_SIZE];
        snprintf(info_file, sizeof(info_file), "%s/info.txt", user_dir);

        FILE *file = fopen(info_file, "w");
        if (file)
        {
            fprintf(file, "ID:%d\nPASSWORD:%s\n", user_id, password);
            fclose(file);
        }
        file = fopen(U_FILE, "a");
        fprintf(file, "%s\n", username);
        fclose(file);
        return user_id;
    }
    return -1;
}

int load_user_name(Client *clients, int max_clients)
{
    int i = 0, user_id;
    FILE *file = fopen(U_FILE, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error: Unable to open user file.\n");
        return -1;
    }
    for (int j = 0; j < MAX_FRIENDS; j++)
    {
        clients[j].id = -1;
        clients[j].is_online = 0;
        clients[j].username[0] = '\0';
        clients[j].password[0] = '\0';
        clients[j].add_friend_requests[i] = -1;
        clients[j].friends[i] = -1;
    }
    char buffer[BUFFER_SIZE];
    while (i < max_clients && fgets(buffer, sizeof(buffer), file) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        char info_file[BUFFER_SIZE];
        snprintf(info_file, sizeof(info_file), "%s/%s/info.txt", BASE_DIR, buffer);
        FILE *inf_file = fopen(info_file, "r");
        if (inf_file == NULL)
        {
            fprintf(stderr, "Error: Unable to open info file for user '%s'.\n", buffer);
            continue;
        }

        char buff[BUFFER_SIZE];
        if (fgets(buff, sizeof(buff), inf_file) != NULL)
        {
            sscanf(buff, "ID:%d", &user_id);
        }
        if (fgets(buff, sizeof(buff), inf_file) != NULL)
        {
            sscanf(buff, "PASSWORD:%s", clients[user_id].password);
        }

        strncpy(clients[user_id].username, buffer, BUFFER_SIZE - 1);
        clients[user_id].username[sizeof(clients[user_id].username) - 1] = '\0';
        clients[user_id].id = user_id;
        fclose(inf_file);
        i++;
    }
    fclose(file);
    return i;
}
