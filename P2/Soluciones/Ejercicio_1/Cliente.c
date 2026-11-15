#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

void comprobarArgumentos(int argc, char *argv[]){
    // Comprueba los argumentos
    if(argc != 2){
        printf("Uso: %s IP_Servidor\n", argv[0]);
        exit(1);
    }
}

int crearSocket(){
    int descriptorCliente = socket(AF_INET, SOCK_STREAM, 0); // Descriptor del canal con el servidor
    if(descriptorCliente == -1){
        perror("Error al crear el conector (socket)");
        exit(1);
    }
    printf("Socket creado.\n");
    return descriptorCliente;
}

// La dirección se pasa por puntero para que los cambios lleguen al main
void configDirServidor(struct sockaddr_in *direccionServidor, char *ip){
    direccionServidor->sin_family = AF_INET; // Familia de protocolos
    direccionServidor->sin_addr.s_addr = inet_addr(ip); // IP del servidor (primer argumento)
    direccionServidor->sin_port = htons(9999); // Puerto del servidor
}

void hacerConnect(int descriptorCliente, struct sockaddr_in direccionServidor){
    // Conecta el canal con la IP y el puerto del servidor
    if(connect(descriptorCliente, (struct sockaddr *)&direccionServidor, sizeof(direccionServidor)) == -1){
        perror("Error al conectar con el servidor (connect)");
        exit(1);
    }
    printf("Socket conectado al servidor con éxito.\n");
}

void recibirDatos(int descriptorCliente){
    char buffer[1024]; // Buffer de lectura de los datos recibidos
    int numBytesLeidos;

    printf("Recibiendo datos...\n");
    // Lee hasta que se cierra la conexión por parte del servidor
    while((numBytesLeidos = read(descriptorCliente, buffer,sizeof(buffer)))>0){
        write(1, buffer, numBytesLeidos); // y lo saca por pantalla.
    }
    if(numBytesLeidos == -1){
        perror("Error al leer del servidor (read)");
    }else{
        printf("Datos recibidos con éxito.\n");
    }
}

int main(int argc, char *argv[]){

    comprobarArgumentos(argc, argv);

    int descriptorCliente = crearSocket();

    struct sockaddr_in direccionServidor; // Estructura dirección del servidor

    configDirServidor(&direccionServidor, argv[1]);

    hacerConnect(descriptorCliente, direccionServidor);

    recibirDatos(descriptorCliente);

    close(descriptorCliente);
    return 0;
}