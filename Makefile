CC=gcc
CFLAGS=-Wall -pthread
CURLFLAGS=-lcurl

# Regla principal: compilar todo
all: prethread-WebServer HTTPclient

# Compilar el servidor
prethread-WebServer: main.o server.o http_manejo.o 
	$(CC) $(CFLAGS) -o prethread-WebServer main.o server.o http_manejo.o 

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

http_manejo.o: http_manejo.c
	$(CC) $(CFLAGS) -c http_manejo.c

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c

# Compilar el cliente HTTP
HTTPclient: HTTPclient.o
	$(CC) -Wall -o HTTPclient HTTPclient.o $(CURLFLAGS)

HTTPclient.o: HTTPclient.c
	$(CC) -Wall -c HTTPclient.c

# Limpiar archivos
clean:
	rm -f *.o prethread-WebServer HTTPclient
