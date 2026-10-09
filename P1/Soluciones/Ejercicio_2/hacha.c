#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
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
    
    if(tamTrozo <= 0){
        perror("Error: tamaño de trozos inferior o igual a 0.");
        exit(1);
    }

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

void codigoPadre(int descEscribirPipe, int descArchivo, int tamTrozo){

    char buffer[1024];
    int pendientes = tamTrozo;   // bytes que aún faltan por enviar de este trozo
    int aLeer, leidos;

    while(pendientes > 0){
        // Leemos como máximo lo que cabe en el buffer, sin pasarnos del trozo
        aLeer = sizeof(buffer);
        if(pendientes < aLeer){ // Si quedan menos bytes que el buffer,
            aLeer = pendientes; // leemos solo los pendientes para no pasarnos del trozo.
        }

        leidos = read(descArchivo, buffer, aLeer); // Bytes leidos
        if(leidos < 0){
            perror("Error al leer el archivo origen");
            exit(1);
        }

        if(leidos == 0){   // Si no hay más datos que enviar, fin de archivo.
            break;
        }

        // si no coinciden con los leidos y los escritos, error.
        if(write(descEscribirPipe, buffer, leidos) != leidos){ 
            perror("Error al escribir en la tubería");
            exit(1);
        }
        pendientes -= leidos;
    }

    close(descEscribirPipe);   // El hijo recibe EOF
}

void codigoHijo(int descLeerPipe, const char *nombreArchivo, int trozo){

    char nombreDestino[512]; // 512 para cubrir todos los posibles nombres de archivo
    char buffer[1024];
    int descDestino, leidos;

    // Nombres: archivo.h00, archivo.h01, ...
    sprintf(nombreDestino, "%s.h%02d", nombreArchivo, trozo);

    // Creamos el archivo destino
    descDestino = creat(nombreDestino, 0666);
    if(descDestino < 0){
        perror("Error al crear el archivo del trozo");
        exit(1);
    }

    // Copiamos de la tubería al archivo hasta recibir EOF
    while((leidos = read(descLeerPipe, buffer, sizeof(buffer))) > 0){
        if(write(descDestino, buffer, leidos) != leidos){ // Si lo leído y lo escrito no 
            perror("Error al escribir el trozo");         // coinciden, error.
            exit(1);
        }
    }

    if(leidos < 0){
        perror("Error al leer de la tubería");
        exit(1);
    }

    close(descDestino);  // Al terminar de copiar se cierran ambos descriptores
    close(descLeerPipe); // para no ocupar espacios en la tabla.
    exit(0); // Muere el hijo
}

void crearPipesyProcesos(const char *nombreArchivo, int tamTrozo, int numTrozos, int descriptorArchivo){
    int descriptoresPipe[2];

    
    for(int trozo = 0; trozo < numTrozos; trozo++){ 
        // Crear tubería
        if(pipe(descriptoresPipe) < 0){ 
            perror("Error al crear la tubería");
            exit(1);
        }

        // Crear proceso
        pid_t pid = fork();

        switch(pid){
            case -1:
                // ERROR
                perror("Error al crear un hijo");
                exit(1);
            case 0:
                // HIJO
                close(descriptoresPipe[1]);
                codigoHijo(descriptoresPipe[0], nombreArchivo, trozo); 
            default:
                // PADRE
                close(descriptoresPipe[0]);
                codigoPadre(descriptoresPipe[1], descriptorArchivo, tamTrozo);
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

    close(descriptorArchivo); // Se cierra el archivo

    // Se espera a todos los hijos
    for(int i=0; i < numTrozos; i++){
        wait(NULL);
    }
    return 0;
}