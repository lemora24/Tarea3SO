#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

FILE *archivo_salida = NULL;

void print_usage(const char *prog_name) {
    printf("Uso: %s -h <host:puerto> -m <metodo> -p <path> [-d <data>] [-o <archivo_salida>]\n", prog_name);
}

// Escribir en archivo si se indicó -o, si no en stdout
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    (void)userdata;
    if (archivo_salida) {
        return fwrite(ptr, size, nmemb, archivo_salida);
    } else {
        return fwrite(ptr, size, nmemb, stdout);
    }
}

int main(int argc, char *argv[]) {
    const char *host = NULL;
    const char *method = NULL;
    const char *path = NULL;
    const char *data = NULL;
    const char *output_file = NULL;

    if (argc < 7) {
        print_usage(argv[0]);
        return 1;
    }
//Ciclo para recibir correctamente los comandos 
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            host = argv[++i];
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            method = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            path = argv[++i];
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            data = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!host || !method || !path) {
        print_usage(argv[0]);
        return 1;
    }

    // Abrir archivo si se pasó -o
    if (output_file) {
        archivo_salida = fopen(output_file, "wb");
        if (!archivo_salida) {
            fprintf(stderr, "Error al abrir archivo de salida: %s\n", output_file);
            return 1;
        }
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error al inicializar libcurl\n");
        return 1;
    }

    char url[2048];
    snprintf(url, sizeof(url), "http://%s%s", host, path);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    //Obtener acción para el protocolo, si no se acepta la accion se devuelve un metodo no soportado
    if (strcmp(method, "GET") == 0) {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    } else if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (data) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    } else if (strcmp(method, "PUT") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if (data) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    } else if (strcmp(method, "DELETE") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    } else if (strcmp(method, "HEAD") == 0) {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    } else {
        fprintf(stderr, "Método no soportado: %s\n", method);
        curl_easy_cleanup(curl);
        return 1;
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Error en la solicitud: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);

    if (archivo_salida) fclose(archivo_salida);

    return 0;
}
