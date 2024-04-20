#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAX_CLIENTS 5
#define DB_FILE "database.txt"

pthread_mutex_t lock; // Mutex for thread-safe access to database

// Structure to represent key-value pairs
typedef struct {
    int key;
    char value[256];
} KeyValuePair;

// Function to handle client requests
void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[1024];
    char reply[256];
    FILE *db_file;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        if (recv(client_socket, buffer, sizeof(buffer), 0) <= 0) {
            break; // Client disconnected
        }

        // Parse client request
        int key, value_len;
        char operation[4];
        char value[256];

        sscanf(buffer, "%s %d %s", operation, &key, value);

        pthread_mutex_lock(&lock); // Lock mutex before accessing database

        if (strcmp(operation, "put") == 0) {
            // Handle put operation
            db_file = fopen(DB_FILE, "a+");
            fseek(db_file, 0, SEEK_SET);

            KeyValuePair kv;
            int key_exists = 0;

            // Check if key already exists
            while (fread(&kv, sizeof(KeyValuePair), 1, db_file) == 1) {
                if (kv.key == key) {
                    key_exists = 1;
                    break;
                }
            }

            if (key_exists) {
                strcpy(reply, "Error: Key already exists");
            } else {
                kv.key = key;
                strcpy(kv.value, value);
                fwrite(&kv, sizeof(KeyValuePair), 1, db_file);
                strcpy(reply, "OK");
            }

            fclose(db_file);
        } else if (strcmp(operation, "get") == 0) {
            // Handle get operation
            db_file = fopen(DB_FILE, "r");
            fseek(db_file, 0, SEEK_SET);

            KeyValuePair kv;
            int key_found = 0;

            while (fread(&kv, sizeof(KeyValuePair), 1, db_file) == 1) {
                if (kv.key == key) {
                    strcpy(reply, kv.value);
                    key_found = 1;
                    break;
                }
            }

            if (!key_found) {
                strcpy(reply, "Error: Key not found");
            }

            fclose(db_file);
        } else if (strcmp(operation, "del") == 0) {
    // Handle delete operation
    db_file = fopen(DB_FILE, "r+");
    if (db_file == NULL) {
        perror("Error opening database file");
        strcpy(reply, "Error: Internal server error");
    } else {
        fseek(db_file, 0, SEEK_SET);

        KeyValuePair kv;
        int key_found = 0;
        int num_entries = 0;
        long pos = 0;
        long read_pos = 0;

        // Count number of entries and find position of matching key
        while (fread(&kv, sizeof(KeyValuePair), 1, db_file) == 1) {
            if (kv.key == key) {
                key_found = 1;
                pos = read_pos; // Position of matching entry to be deleted
            }
            read_pos = ftell(db_file); // Get current file position
            num_entries++;
        }

        if (!key_found) {
            strcpy(reply, "Error: Key not found");
        } else {
            // Move entries after the deleted entry
            for (int i = pos / sizeof(KeyValuePair) + 1; i < num_entries; i++) {
                fseek(db_file, i * sizeof(KeyValuePair), SEEK_SET);
                fread(&kv, sizeof(KeyValuePair), 1, db_file);
                fseek(db_file, (i - 1) * sizeof(KeyValuePair), SEEK_SET);
                fwrite(&kv, sizeof(KeyValuePair), 1, db_file);
            }

            // Truncate the file to remove the last entry
            ftruncate(fileno(db_file), (num_entries - 1) * sizeof(KeyValuePair));

            strcpy(reply, "OK");
        }

        fclose(db_file);
    }
}

        pthread_mutex_unlock(&lock); // Unlock mutex after accessing database

        // Send reply to client
        send(client_socket, reply, strlen(reply), 0);

        // Check for termination condition
        if (strcmp(operation, "Bye") == 0) {
            strcpy(reply,"Goodbye");
            send(client_socket, reply, strlen(reply), 0);
            // Close client socket
            close(client_socket);
            free(arg);
            return NULL;
            break;
        }else{
            strcpy(reply,"Error: Invalid command");
        }
    }

    // Close client socket
    close(client_socket);
    free(arg);
    return NULL;
}

int main() {
    int listen_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t tid;

    // Create socket
    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8080); // Use port 8080

    // Bind socket to address and port
    if (bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listen_socket, MAX_CLIENTS) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 8080...\n");

    // Initialize mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        perror("mutex initialization failed");
        exit(EXIT_FAILURE);
    }

    // Accept incoming connections and handle them concurrently
    while (1) {
        // Accept connection from client
        if ((client_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &client_len)) == -1) {
            perror("accept failed");
            continue;
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Create a new thread to handle client
        int *client_sock = malloc(sizeof(int));
        *client_sock = client_socket;

        if (pthread_create(&tid, NULL, handle_client, (void *)client_sock) != 0) {
            perror("pthread_create failed");
            continue;
        }

        // Detach thread to avoid memory leaks
        pthread_detach(tid);
    }

    // Close the listening socket
    close(listen_socket);

    // Destroy mutex
    pthread_mutex_destroy(&lock);

    return 0;
}
