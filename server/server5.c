#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h> 
#include <sys/stat.h>

#define PORT 9989
#define BUFF_SIZE 512
#define FILE_BUFF_SIZE 1024
#define MAX_CLIENTS 100
#define MAX_GROUP 100

char sys_path[] = "/mnt/disk3/tien_aiot/file_sharing-/server/db";

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;


// utils.c
void create_folder_if_not_exist(const char *folder_path) {
    // Check if the folder exists
    struct stat st = {0};
    if (stat(folder_path, &st) == -1) {
        // If not, create the folder
        mkdir(folder_path, 0777);
    }
}

// utils.c
int check_username_existence(const char *username) {
    FILE *file = fopen("users.txt", "r");
    if (file != NULL) {
        char line[BUFF_SIZE];
        while (fgets(line, sizeof(line), file) != NULL) {
            char stored_username[50];
            sscanf(line, "%s", stored_username);

            if (strcmp(username, stored_username) == 0) {
                fclose(file);
                return 1; // Username exists
            }
        }
        fclose(file);
    }
    return 0; // Username does not exist
}
// utils.c
const char *extract_filename(const char *path) {
    const char *last_slash = strrchr(path, '/');  // Find the last occurrence of '/'
    
    if (last_slash != NULL) {
        return last_slash + 1;  // Return the part after the last '/'
    } else {
        return path;  // No '/' found, return the original path
    }
}
// utils.c
int is_member(const char *username, const char *groupname) {
    FILE *file = fopen("group_members.txt", "r");
    char line[1024];
    char group_name[20], member[20];

    if (file != NULL) {
        while (fscanf(file, "%s %s", group_name, member) != EOF) {
            if (strcmp(username, member) == 0 && strcmp(groupname, group_name) == 0) {
                fclose(file);
                return 1;
            }
        }
        fclose(file);
    }
    return 0;
}

// utils.c
void send_message(int client_socket, const char *message) {
    send(client_socket, message, strlen(message), 0);
}

void handle_user_registration(int client_socket, char *command) {
    // Implement user registration logic
    // Read user data from socket and save to 'users.txt'
    char REG[10] = {0};
    char username[20] = {0};
    char password[20] = {0};
    sscanf(command, "%s %s %s", REG, username, password);

    if (check_username_existence(username)) {
        // Username already exists, send a message to the client
        send_message(client_socket, "-1");
    } else {
        // Username does not exist, append buffer to file
        FILE *file = fopen("users.txt", "a");
        if (file != NULL) {
            fputs(username, file);
            fputs(" ", file); // Add a space between username and password
            fputs(password, file);
            fputs("\n", file);
            fclose(file);

            // Send success message to the client
            send_message(client_socket, username );
        } else {
            // Error opening the file
            send_message(client_socket, "0");
        }
    }

}

void handle_user_login(int client_socket, char *command) {
    // Implement user login logic
    
    // Send success or failure response to the client

    char LOG[10] = {0};
    char username[20];
    char password[20];
    sscanf(command, "%s %s %s", LOG, username, password);   
    // Check credentials against 'users.txt' 
    FILE *file = fopen("users.txt", "r");
    if (file != NULL) {
        char line[BUFF_SIZE];
        while (fgets(line, sizeof(line), file) != NULL) {
            char stored_username[20];
            char stored_password[20];
            sscanf(line, "%s %s", stored_username, stored_password);

            if (strcmp(username, stored_username) == 0 && strcmp(password, stored_password) == 0) {
                fclose(file);
                send_message(client_socket, username);
                return;
            }
        }
        fclose(file);
    }
    send_message(client_socket, "-1");
}

void handle_group_creation(int client_socket, char *command) {
    // Implement group creation logic
    // Save group data to 'groups.txt' and 'group_members.txt'

    char CREATE_GROUP[20] = {0};
    char group_name[20] = {0};
    char username[20] = {0};
    sscanf(command, "%s %s %s", CREATE_GROUP, username, group_name);

    FILE *file = fopen("groups.txt", "a");
    if (file != NULL) {
        fputs(group_name, file);
        fputs(" ", file);
        fputs(username, file);
        fputs("\n", file);
        fclose(file);
    }

    file = fopen("group_members.txt", "a");
    if (file != NULL) {
        fputs(group_name, file);
        fputs(" ", file);
        fputs(username, file);
        fputs("\n", file);
        fclose(file);
    }

}

// void get_groups_not_joined(char *username, char ** groups_not_joined) {
//     // Read group_member.txt file to get groups that the user has not joined
//     // Send the list of groups to the client
// } 

void get_groups_not_joined(const char *username, char *groups_not_joined) {

    FILE *groups_file = fopen("groups.txt", "r");
    FILE *members_file = fopen("group_members.txt", "r");
    char line[1024];
    char group_name[20], member[20], creator[20];
    int is_member;

    if (groups_file == NULL || members_file == NULL) {
        strcpy(groups_not_joined, "Error");
        return;
    }



    while (fscanf(groups_file, "%s %s", group_name, creator) != EOF) {
        is_member = 0;
        printf("checking group %s\n", group_name);
        rewind(members_file); // Reset the file pointer to the beginning for each group

        // Skip the header line in group_members.txt
        // fgets(line, sizeof(line), members_file);
        char group_name_tmp[20]={0};
        while (fscanf(members_file, "%s %s", group_name_tmp, member) != EOF) {
            // if (strcmp(username, member) == 0 && strcmp(username, creator) != 0) {
            if (strcmp(username, member) == 0 && strcmp(group_name_tmp, group_name)==0 ) {
                is_member = 1;
                printf("check member %s\n", member) ;
                break;
            }
        }

        if (!is_member) {

            strcat(groups_not_joined, group_name);
            strcat(groups_not_joined, "\n");
        }
        printf("result: %d\n", is_member );
    }

    fclose(groups_file);
    fclose(members_file);
}



void handle_join_group(int client_socket) {
    return ;
}


void handle_file_upload(int client_socket, const char *command) {
    // Implement file upload logic
    // Save files to the respective group folder
    char group_name[20] = {0}; char filename[BUFF_SIZE] = {0};
    char user_name[BUFF_SIZE] = {0};
    sscanf(command, "%*s %s %s %s", group_name, filename, user_name); // Extract group name from the command
    
    // Check if the user is a member of the group
    if (!is_member(user_name, group_name)) {
        send_message(client_socket, "0");
        return;
    }
    

    char folder_path[BUFF_SIZE] = {0}; 
    snprintf(folder_path, sizeof(folder_path), "%s/%s", sys_path, group_name);
    create_folder_if_not_exist(folder_path);
    
    const char *extracted_filename = extract_filename(filename);

    char file_path[BUFF_SIZE] = {0};

    snprintf(file_path, sizeof(file_path), "%s/%s/%s", sys_path, group_name, extracted_filename);

    
    send_message(client_socket, "1");
    printf("Start upload\n");
    
    long file_size;
    recv(client_socket, &file_size, sizeof(file_size), 0);

    
    printf("Received file size: %ld\n", file_size);
    int bytes_received = 0;
    char file_buffer[FILE_BUFF_SIZE] = {0};
    while (bytes_received < file_size) 
    {
        int bytes = recv(client_socket, file_buffer, FILE_BUFF_SIZE, 0);
        if (bytes > 0) {
            FILE *file = fopen(file_path, "ab");
            fwrite(file_buffer, sizeof(char), bytes, file);
            fclose(file);
            bytes_received += bytes;
            memset(file_buffer, 0, sizeof(file_buffer));
        }
        else {
            break;
        }

    }

    // write to files_info.txt
    printf("write to files_info.txt\n");

    memset(file_path, 0, sizeof(file_path));
    snprintf(file_path, sizeof(file_path), "%s/%s/%s", sys_path, group_name, "files_info.txt");
    FILE *file = fopen(file_path, "a");
    if (file != NULL) {
        time_t raw_time;
        struct tm *time_info;
        time(&raw_time);
        time_info = localtime(&raw_time);
        char time_str[20] = {0};
        strftime(time_str, sizeof(time_str), "%d/%m/%Y|%H:%M:%S", time_info);
        fprintf(file, "%s %s %s\n", extracted_filename, user_name, time_str);
        fclose(file);
    }
    printf("Done\n");
}

void handle_file_download(int client_socket) {
    // Implement file download logic
    // Send files from the respective group folder
    char filename[BUFF_SIZE];
    recv(client_socket, filename, BUFF_SIZE, 0);

    FILE *file = fopen(filename, "rb");
    char file_buffer[FILE_BUFF_SIZE];

    if (file != NULL) {
        int bytes_read;
        while ((bytes_read = fread(file_buffer, sizeof(char), FILE_BUFF_SIZE, file)) > 0) {
            send(client_socket, file_buffer, bytes_read, 0);
        }
        fclose(file);
    }
}

void get_groups_joined(int client_socket,const char *command){
    char username[20] = {0};
    sscanf(command, "%*s %s", username);
    char list_of_groups[(MAX_GROUP + 1) * 20] = {0};
    FILE *file = fopen("group_members.txt", "r");
    char line[1024]; char group_name[20], member[20];

    while(fscanf(file, "%s %s", group_name, member) != EOF ) {
        if (strcmp(username, member) == 0) {
            strcat(list_of_groups, group_name);
            strcat(list_of_groups, "\n");
        }
    }

    fclose(file);
    send_message(client_socket, list_of_groups);

}

void * process_client_request(void *arg) {
    int client_socket = *((int*)arg);
    free(arg);

    char command[BUFF_SIZE] = {0};

    while(1) {
        memset(command, 0, BUFF_SIZE);
        
        printf("Waiting for command...\n");
        
        int bytes_read = recv(client_socket, command, BUFF_SIZE - 1, 0);
        if (bytes_read <= 0) return 0;

        // command[bytes_read] = '\0';

        printf("Received command: %s\n", command);

        if (strncmp(command, "REG", 3) == 0) {
            handle_user_registration(client_socket, command);
        } else if (strncmp(command, "LOGIN", 5) == 0) {
            handle_user_login(client_socket, command);
        } else if (strncmp(command, "CREATE_GROUP", 12) == 0) {
            handle_group_creation(client_socket, command);
        } else if (strncmp(command, "NOT_JOINED_GROUP", 16) == 0) {
            char username[20] = {0};
            sscanf(command, "%*s %s", username); // Extract username from the command

            char groups_not_joined[(MAX_GROUP + 1) * 20] = {0};
            get_groups_not_joined(username, groups_not_joined);

            if (strcmp(groups_not_joined, "Error") == 0) {
                send_message(client_socket, "Error retrieving group list");
            } else if (strlen(groups_not_joined) == 0) {
                send_message(client_socket, "No available groups to join");
            } else {
                send_message(client_socket, groups_not_joined); // Send list of groups to the client
            }

        } else if (strncmp(command, "JOIN_GROUP", 10) == 0) {
            // TODO
        } else if (strncmp(command, "LEAVE_GROUP", 11) == 0) {
            // TODO
        } else if (strncmp(command, "YOUR_GROUPS", 11) == 0) {
            get_groups_joined(client_socket, command);

        } else if (strncmp(command, "UPLOAD", 6) == 0) {
            
            handle_file_upload(client_socket, command);
        } else if (strncmp(command, "DOWNLOAD_FILE", 13) == 0) {
            handle_file_download(client_socket);
        } else if (strncmp(command, "QUIT", 4) == 0) {
            printf("Client disconnected\n");
            return 0;
        } else {
            printf("Invalid command\n");

        }
    }
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Bind the socket to the specified port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for client connections
    listen(server_fd, MAX_CLIENTS);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        printf("\nNew connection\n");

        pthread_t pid;
        int *arg = (int *)calloc(1, sizeof(int));
        *arg = client_socket;
        pthread_create(&pid, NULL, process_client_request, (void*)arg);
    }
    close(server_fd);
    return 0;
}


// Kiểm soát quyền truy cập ??? 
// Xử lý file có kích thước lớn bất kỳ ??? 
// Liệt kê danh sách nhóm 
// Liệt kê danh sách thành viên trong nhóm
// Yêu cầu tham gia một nhóm và phê duyệt                           
// Thoát khỏi một nhóm
// Mời tham gia một nhóm và phê duỵet, Xóa thành viên khỏi nhóm     // TODO 

// Download file từ group // TODO

// Liệt kê nội dung thư mục 
// Ghi log 
