import subprocess
import threading
import sys
import time

##Funcion para ejecutar cliente http
def lanzar_httpclient(host, path, method):
    try:
        subprocess.run(["./HTTPclient", "-h", host, "-m", method, "-p", path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except Exception as e:
        print(f"Error al lanzar HTTPclient: {e}")

def main():
    if len(sys.argv) != 5:
        print("Uso: python3 stress.py -n <hilos> -h <host:puerto>")
        print("Ejemplo: python3 stress.py -n 100 -h localhost:8080")
        sys.exit(1)

    num_hilos = 0
    host = None

    for i in range(1, len(sys.argv)):
        if sys.argv[i] == "-n" and i + 1 < len(sys.argv):
            num_hilos = int(sys.argv[i + 1])
        if sys.argv[i] == "-h" and i + 1 < len(sys.argv):
            host = sys.argv[i + 1]

    if num_hilos <= 0 or host is None:
        print("Parámetros inválidos.")
        sys.exit(1)

    print(f"Iniciando ataque de {num_hilos} conexiones contra {host}")

    hilos = []
    inicio = time.time()

    for _ in range(num_hilos):
        t = threading.Thread(target=lanzar_httpclient, args=(host, "/", "GET"))
        t.start()
        hilos.append(t)

    for t in hilos:
        t.join()

    fin = time.time()
    print(f"Completado en {fin - inicio:.2f} segundos")

if __name__ == "__main__":
    main()
