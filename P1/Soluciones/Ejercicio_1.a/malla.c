#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

pid_t pid_Padre; // PID SUPERPADRE

void despertar(int sig){} // MANEJADOR VACÍO

// GESTIÓN DE ARGUMENTOS
void parse(int argc, char *argv[]){
    if(argc != 3){
        printf("ERROR: número de argumentos incorrectos\n");
        exit(1);
    }
}

// PSTREE
void funcion_pstree(){
    if(fork()==0){
        // HIJO
        char strPidP[10];
        sprintf(strPidP, "%d", pid_Padre);
        execlp("pstree","pstree","-c", strPidP ,NULL);

        exit(0);
    }
}

// CREA LAS COLUMNAS DE HIJOS 
void vertical(int columnaActual,int numFila, int numCol){
    int filaActual = 0;

    // CREACIÓN COLUMNAS (f=1 porque la primera fila ya la ha creado el superpadre)
    for(int f=1; f<numFila; f++){
        if(fork() != 0){ // Si es el padre sale del bucle de creación
            break;
        }
        filaActual = f;
    }
    // SEÑALES 
    signal(SIGUSR1, despertar);

    // Último hijo
    if(filaActual == (numFila-1) && columnaActual == (numCol-1)){
            kill(pid_Padre, SIGUSR1); // Manda la señal al padre para hacer pstree
    }

    pause(); // Todos los hijos esperan a la señal en cascada del superpadre

    wait(NULL); // Todos esperan a que sus hijos mueran
    exit(0); // y muere.
}

// CREA LA PRIMERA FILA DE PROCESOS
void horizontal(int numFila, int numCol){
    for(int c=0; c<numCol; c++){
        if(fork() == 0){  // Si es un hijo crea su columna, si no es el padre y sigue creando la primera fila.
            vertical(c, numFila, numCol);
        }
    }
}

int main(int argc, char *argv[]){
    // ARGUMENTOS
    parse(argc, argv);
    int numFila = atoi(argv[1]);
    int numCol = atoi(argv[2]);

    pid_Padre = getpid(); // Guardamos el pid del padre

    horizontal(numFila, numCol);

    signal(SIGUSR1, despertar); // Configurar señal
    pause(); // Espera la señal que le manda el último hijo

    funcion_pstree();
    wait(NULL); // Espera a que pstree termine

    kill(0, SIGUSR1); // Manda señal en cascada

    // Espera por todos los hijos que tenga
    for (int i = 0; i < numCol; i++) { 
        wait(NULL);
    }

    return 0;
}