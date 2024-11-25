#include "user_manager.h"

int next_id = 0;

void load_next_id() {
    FILE *file = fopen(ID_FILE, "r");
    if (file) {
        fscanf(file, "%d", &next_id);
        fclose(file);
    } else {
        next_id = 0;  
    }
}

void save_next_id() {
    FILE *file = fopen(ID_FILE, "w");
    if (file) {
        fprintf(file, "%d", next_id);
        fclose(file);
    }
}

int create_user_directory(const char *username, const char *password) {
    char user_dir[BUFFER_SIZE];
    snprintf(user_dir, sizeof(user_dir), "%s/%s", BASE_DIR, username);

    if (mkdir(user_dir, 0700) == 0) {
        int user_id = next_id++;
        save_next_id();  

        char info_file[BUFFER_SIZE];
        snprintf(info_file, sizeof(info_file), "%s/info.txt", user_dir);

        FILE *file = fopen(info_file, "w");
        if (file) {
            fprintf(file, "ID:%d\nPASSWORD:%s", user_id, password);
            fclose(file);
        }
        file = fopen(U_FILE, "a");
        fprintf(file, "%s\n", username);
        fclose(file);
        return user_id;
    }
    return -1;
}


int load_user_name(Client *clients, int max_clients) {
    int i = 0;
    FILE *file = fopen(U_FILE, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open user file.\n");
        return -1; 
    }
    char buffer[BUFFER_SIZE];
    while (i < max_clients && fgets(buffer, sizeof(buffer), file) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';

        char info_file[BUFFER_SIZE];
        snprintf(info_file, sizeof(info_file), "%s/%s/info.txt", BASE_DIR, buffer);
        FILE *inf_file = fopen(info_file, "r");
        if (inf_file == NULL) {
            fprintf(stderr, "Error: Unable to open info file for user '%s'.\n", buffer);
            continue; 
        }

        clients[i].id = 0;
        clients[i].username[0] = '\0';
        clients[i].password[0] = '\0';

        char buff[BUFFER_SIZE];
        if (fgets(buff, sizeof(buff), inf_file) != NULL) {
            sscanf(buff, "ID:%d", &clients[i].id);
        }
        if (fgets(buff, sizeof(buff), inf_file) != NULL) {
            sscanf(buff, "PASSWORD:%s", clients[i].password); 
        }

        strncpy(clients[i].username, buffer, sizeof(clients[i].username) - 1);
        clients[i].username[sizeof(clients[i].username) - 1] = '\0';

        fclose(inf_file);
        i++;
    }
    fclose(file);
    return i;
}

