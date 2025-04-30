#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

#define BUFFER_SIZE 4096

//Metodo para manejar solicitudes de diferentes protocolos
void handle_request(int client_socket, const char *root_dir, const char *protocol) {
    // Si el protocolo NO es HTTP, simular otros protocolos
    if (strcmp(protocol, "HTTP") != 0) {
        if (strcmp(protocol, "FTP") == 0) {
            dprintf(client_socket, "220 (Fake FTP Server Ready)\r\n");
        } else if (strcmp(protocol, "SSH") == 0) {
            dprintf(client_socket, "SSH-2.0-FakeSSH_1.0\r\n");
        } else if (strcmp(protocol, "SMTP") == 0) {
            dprintf(client_socket, "220 (Fake SMTP Server Ready)\r\n");
        } else if (strcmp(protocol, "DNS") == 0) {
            dprintf(client_socket, "Fake DNS Response\r\n");
        } else if (strcmp(protocol, "TELNET") == 0) {
            dprintf(client_socket, "Fake Telnet Server Ready\r\n");
        } else if (strcmp(protocol, "SNMP") == 0) {
            dprintf(client_socket, "Fake SNMP Response\r\n");
        } else {
            dprintf(client_socket, "500 Unknown Protocol\r\n");
        }
        return;
    }

    //comportamiento normal HTTP

    char buffer[BUFFER_SIZE];
    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);

    if (bytes_read <= 0) {
        return;
    }

    buffer[bytes_read] = '\0';

    // Parsear método y ruta
    char method[8];
    char path[1024];

    if (sscanf(buffer, "%s %s", method, path) != 2) {
        dprintf(client_socket, "HTTP/1.1 400 Bad Request\r\n\r\n");
        return;
    }

    // Flags de método
    int is_head = 0;
    int is_post = 0;
    int is_put = 0;
    int is_delete = 0;

    if (strcmp(method, "GET") == 0) {
        // ok
    } else if (strcmp(method, "HEAD") == 0) {
        is_head = 1;
    } else if (strcmp(method, "POST") == 0) {
        is_post = 1;
    } else if (strcmp(method, "PUT") == 0) {
        is_put = 1;
    } else if (strcmp(method, "DELETE") == 0) {
        is_delete = 1;
    } else {
        dprintf(client_socket, "HTTP/1.1 501 Not Implemented\r\n\r\n");
        return;
    }

    // Armar ruta completa
    char file_path[2048];
    snprintf(file_path, sizeof(file_path), "%s%s", root_dir, path);
    if (strcmp(path, "/") == 0 && !is_post && !is_put && !is_delete) {
        strcat(file_path, "index.html");
    }

    // DELETE eliminar archivo
    if (is_delete) {
        if (access(file_path, F_OK) != 0) {
            dprintf(client_socket, "HTTP/1.1 404 Not Found\r\n\r\n");
            return;
        }

        if (remove(file_path) != 0) {
            dprintf(client_socket, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
            return;
        }

        dprintf(client_socket, "HTTP/1.1 200 OK\r\n\r\n");
        return;
    }

    // POST y PUT: guardar archivo
    if (is_post || is_put) {
        char *body = strstr(buffer, "\r\n\r\n");
        if (body != NULL) {
            body += 4;

            FILE *f = fopen(file_path, "w");
            if (f == NULL) {
                dprintf(client_socket, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
                return;
            }

            fwrite(body, 1, strlen(body), f);
            fclose(f);

            if (is_post) {
                dprintf(client_socket, "HTTP/1.1 201 Created\r\n\r\n");
            } else {
                dprintf(client_socket, "HTTP/1.1 200 OK\r\n\r\n");
            }
        } else {
            dprintf(client_socket, "HTTP/1.1 400 Bad Request\r\n\r\n");
        }

        return;
    }

    // GET o HEAD servir archivo
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        dprintf(client_socket, "HTTP/1.1 404 Not Found\r\n\r\n");
        return;
    }

    struct stat st;
    fstat(fd, &st);

    dprintf(client_socket,
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n\r\n", st.st_size);

    if (!is_head) {
        sendfile(client_socket, fd, NULL, st.st_size);
    }

    close(fd);
}
