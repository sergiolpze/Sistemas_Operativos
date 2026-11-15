#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

int crearSocket(){
    int descriptorServidor = socket(AF_INET,SOCK_STREAM,0);
    if(descriptorServidor == -1){
        perror("Error al crear el conector (socket)");
        exit(1);
    }
    printf("Socket creado.\n");
    return descriptorServidor;
}

// La dirección se pasa por puntero para que los cambios lleguen al main
void configDirServidor(struct sockaddr_in *direccionServidor){
    direccionServidor->sin_family = AF_INET; // Familia de protocolos
    direccionServidor->sin_addr.s_addr = htonl(INADDR_ANY); // Dirección IP a escuchar, cualquiera en este caso
    direccionServidor->sin_port = htons(9999); // Puerto de escucha del servidor
    printf("Dirección del servidor configurada.\n");
}

void hacerBind(int descriptorServidor, struct sockaddr_in direccionServidor){
    // Asocia el conector a la dirección y puerto del servidor
    if(bind(descriptorServidor, (struct sockaddr *)&direccionServidor, sizeof(direccionServidor)) == -1){
        perror("Error al asociar el conector a la dirección del servidor (bind)");
        exit(1);
    }
    printf("Asociación del socket y de la dirección del servidor realizada.\n");
}

void hacerListen(int descriptorServidor){
    // Pone el conector a la espera, y se almacenan en una cola hasta 10 peticiones
    if(listen(descriptorServidor, 10) == -1){
        perror("Error en el listen");
        exit(1);
    }
    printf("Servidor en escucha(listen). A la espera de peticiones...\n");
}

int aceptarPeticion(int descriptorServidor){
    struct sockaddr_in direccionCliente; // Estructura dirección del cliente (la rellena accept)
    socklen_t tamDirCliente = sizeof(direccionCliente); // Tamaño de la dirección del cliente

    // Descriptor del socket del cliente
    int descriptorCliente = accept(descriptorServidor, (struct sockaddr *)&direccionCliente, &tamDirCliente); 
    if(descriptorCliente == -1){
        perror("Error al aceptar la petición de un cliente");
        exit(1);
    }
    printf("¡Petición recibida y aceptada!\n");
    return descriptorCliente;
}

int abrirArchivo(){
    int descriptorArchivo = open("Google.html", O_RDONLY);
    if(descriptorArchivo == -1){
        perror("Error al abrir el archivo.");
        exit(1);
    }
    printf("Archivo abierto con éxito.\n");
    return descriptorArchivo;
}

void mandarDatos(int descriptorArchivo, int descriptorCliente){
    int numBytesLeidos;
    char buffer[1024]; // Buffer de lectura del archivo
    
    // Lee hasta el final del archivo
    while((numBytesLeidos = read(descriptorArchivo, buffer,sizeof(buffer)))>0){ 
        write(descriptorCliente, buffer, numBytesLeidos); // y lo manda al cliente.
    }
    printf("Finalizada la lectura-escritura del archivo al cliente.\n");
}

void codigoHijo(int descriptorServidor, int descriptorCliente){
    close(descriptorServidor); // El hijo no necesita el conector de escucha
    printf("Fork realizado. Hijo creado con éxito.\n");

    int descriptorArchivo = abrirArchivo();

    mandarDatos(descriptorArchivo, descriptorCliente);

    close(descriptorArchivo);
    close(descriptorCliente);

    printf("Soy el hijo. He cumplido mi cometido y muero.\n");
    exit(0);
}

int main(){

    int descriptorServidor = crearSocket();

    struct sockaddr_in direccionServidor; // Estructura dirección del servidor

    configDirServidor(&direccionServidor);
    hacerBind(descriptorServidor, direccionServidor);
    hacerListen(descriptorServidor);

    while(1){
        int descriptorCliente = aceptarPeticion(descriptorServidor);

        switch(fork()){
            case -1:
                perror("Error al crear el proceso hijo");
                close(descriptorCliente);
                exit(1);

            case 0:     // HIJO
                codigoHijo(descriptorServidor, descriptorCliente);
                break;

            default:    // PADRE
                close(descriptorCliente);
                break;
        }
    }
}