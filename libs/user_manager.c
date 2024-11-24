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
        return user_id;
    }
    return -1;
}
