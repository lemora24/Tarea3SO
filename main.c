#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declaraciones directas
void start_server(int thread_count, const char *root_dir, int port, const char *protocol);

// Variable global de protocolo
const char *global_protocol = "HTTP";  // Por defecto

int main(int argc, char *argv[]) {
    if (argc < 7) {
        fprintf(stderr, "Uso: %s -n <hilos> -w <root> -p <puerto> [-proto <protocolo>]\n", argv[0]);
        return 1;
    }

    int thread_count = 0;
    int port = 0;
    const char *root = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            thread_count = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            root = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-proto") == 0 && i + 1 < argc) {
            global_protocol = argv[++i];
        } else {
            fprintf(stderr, "Parámetro no reconocido: %s\n", argv[i]);
            return 1;
        }
    }

    if (thread_count <= 0 || port <= 0 || root == NULL) {
        fprintf(stderr, "Parámetros inválidos.\n");
        return 1;
    }

    start_server(thread_count, root, port, global_protocol);

    return 0;
}
