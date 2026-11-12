#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>


int main(){
    int descriptorServidor = socket(AF_INET,SOCK_STREAM,0);
    if(descriptorServidor == -1){
        perror("Error al crear el conector (socket)");
        exit(1);
    }

    int descriptorCliente;
    struct sockaddr_in direccionServidor; // Estructura dirección del servidor
    struct sockaddr_in direccionCliente; // Estructura dirección del cliente
    socklen_t tamDirCliente; // Tamaño de la dirección del cliente.

    int descriptorArchivo; 
    char buffer[1024]; // Buffer de lectura del archivo
    int numBytesLeidos; 
    

    direccionServidor.sin_family = AF_INET; // Familia de protocolos
    direccionServidor.sin_addr.s_addr = inet_addr("127.0.0.1"); // Dirección IP a escuchar
    direccionServidor.sin_port = htons(9999); // Puerto de escucha del servidor

    // Asocia el conector a la dirección y puerto del servidor
    if(bind(descriptorServidor, (struct sockaddr *)&direccionServidor, sizeof(direccionServidor)) == -1){
        perror("Error al asociar el conector a la dirección del servidor (bind)");
        exit(1);
    }

    // Pone el conector a la espera
    if(listen(descriptorServidor, 10) == -1){
        perror("Error al escuchar (listen)");
        exit(1);
    }

    while(1){

        tamDirCliente = sizeof(direccionCliente);

        descriptorCliente = accept(descriptorServidor, (struct sockaddr *)&direccionCliente, &tamDirCliente); // Descriptor del socket del cliente
        if(descriptorCliente == -1){
            perror("Error al aceptar la petición de un cliente");
            exit(1);
        }

        switch(fork()){
            case -1:
                perror("Error al crear el proceso hijo");
                exit(1);

            case 0:     // HIJO
                descriptorArchivo = open("Google.html", O_RDONLY);
                if(descriptorArchivo == -1){
                    perror("Error al abrir el archivo.");
                    exit(1);
                }
                while(numBytesLeidos = read(descriptorArchivo, buffer,sizeof(buffer))){ // Lee hasta el final del archivo
                    write(descriptorCliente, buffer, numBytesLeidos); // y lo manda al cliente.
                }

                close(descriptorArchivo);
                close(descriptorCliente);
                
                exit(0);

            default:    // PADRE
                close(descriptorCliente);
                break;

        }
    }

}