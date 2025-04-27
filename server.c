#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// Declaración para evitar archivo .h
void handle_request(int client_socket, const char *root_dir);

#define MAX_QUEUE 100

static int server_socket;
static const char *root_dir;
pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_worker(void *arg) {
    struct sockaddr_in client;
    socklen_t client_len;

    while (1) {
        printf("[Hilo] Esperando cliente...\n");

        client_len = sizeof(client);  // 👈 CORRECTO: reiniciar tamaño antes de cada accept()

        pthread_mutex_lock(&accept_mutex);
        int client_socket = accept(server_socket, (struct sockaddr*)&client, &client_len);
        pthread_mutex_unlock(&accept_mutex);

        if (client_socket < 0) {
            perror("Error en accept");
            continue;
        }

        printf("[Hilo] Cliente conectado, manejando solicitud...\n");

        handle_request(client_socket, root_dir);
        close(client_socket);

        printf("[Hilo] Cliente desconectado.\n");
    }

    return NULL;
}


void start_server(int thread_count, const char *dir, int port) {
    root_dir = dir;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error al crear el socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Error al hacer bind");
        exit(1);
    }

    if (listen(server_socket, MAX_QUEUE) < 0) {
        perror("Error al escuchar");
        exit(1);
    }

    printf("Servidor escuchando en el puerto %d con %d hilos disponibles.\n", port, thread_count);

    pthread_t threads[thread_count];
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&threads[i], NULL, thread_worker, NULL) != 0) {
            fprintf(stderr, "Error al crear el hilo %d\n", i);
            exit(1);
        }
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    close(server_socket);
}
