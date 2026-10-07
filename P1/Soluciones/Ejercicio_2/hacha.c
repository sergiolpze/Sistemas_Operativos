#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

// ANALISIS ARGUMENTOS
void parse(int argc, char *argv[]){
    if(argc != 3){
        printf("ERROR: número de argumentos incorrectos.\n");
        exit(1);
    }
}

int calcularNumeroTrozos(const char *nombreArchivo, int tamTrozo){
    struct stat statsArchivo;
    int tamArchivo, numTrozos;
    
    if(stat(nombreArchivo, &statsArchivo) < 0){

        perror("Error al calcular el tamaño del archivo.(stat)\n");
        exit(1);
    }else{
        tamArchivo = statsArchivo.st_size;
        numTrozos = tamArchivo/tamTrozo;
        if(tamArchivo%tamTrozo != 0){ // Si la división no es exacta se añade un trozo más
            numTrozos++;              // para leer lo restante.
        }
    }
    return numTrozos;
}

int abrirArchivo(const char *nombreArchivo){

    int descriptorArchivo = open(nombreArchivo, O_RDONLY);
    if(descriptorArchivo < 0){
        perror("Error al abrir el archivo.\n");
        exit(1);
    }

    return descriptorArchivo;
}

crearPipesyProcesos(const char *nombreArchivo, int tamTrozo, int numTrozos, int descriptorArchivo){
    int descriptoresPipe[2];

    
    for(int i = 0; i < numTrozos; i++){ 
        // Crear tubería
        if(pipe(descriptoresPipe) < 0){ 
            perror("Error al crear la tubería");
            exit(1);
        }

        // Crear proceso
        pid_t pid = fork();

        switch(pid){
            case -1:
                exit(1);
            case 0:
                codigoHijo(); // tarea próxima pasar argumentos a ambos y crear funciones
            default:
                codigoPadre();
        }
    }
}

int main(int argc, char *argv[]){

    parse(argc, argv); // Analisis argumentos

    char *nombreArchivo = argv[1]; // Nombre del archivo
    int tamTrozo = atoi(argv[2]); // Tamaño de los trozos en bytes

    int numTrozos = calcularNumeroTrozos(nombreArchivo, tamTrozo); // Calculamos numero de trozos
    int descriptorArchivo = abrirArchivo(nombreArchivo); // Abrimos y guardamos el descriptor

    crearPipesyProcesos(nombreArchivo, tamTrozo, numTrozos, descriptorArchivo);

    return 0;
}