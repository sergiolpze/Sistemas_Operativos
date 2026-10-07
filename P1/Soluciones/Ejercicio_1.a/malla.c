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
    int filaActual;
    pid_t pidHijo = 0; 

    // CREACIÓN COLUMNAS (f=1 porque la primera fila ya la ha creado el superpadre)
    for(filaActual=1; filaActual<numFila; filaActual++){
        pidHijo = fork(); // Se guarda el pid del hijo
        if(pidHijo > 0){ // Si es el padre se guarda el numero de fila y sale del bucle
            break;
        }
    }
    // SEÑALES 
    signal(SIGUSR1, despertar);

    // Último hijo
    if(filaActual == numFila && columnaActual == (numCol-1)){
            kill(pid_Padre, SIGUSR1); // Manda la señal al padre para hacer pstree
    }

    pause(); // Todos los hijos esperan a la señal en cascada del superpadre
    

    if(pidHijo > 0){ // Si no es el último de la columna manda el kill hacia abajo y espera
        kill(pidHijo, SIGUSR1);
        wait(NULL);
    }
    
    exit(0);
}

// CREA LA PRIMERA FILA DE PROCESOS
void horizontal(int numFila, int numCol, pid_t pidsHijos[]){
    for(int c=0; c<numCol; c++){

        pid_t pidAux = fork();
        if(pidAux == 0){  // Si es un hijo crea su columna, si no es el padre y sigue creando la primera fila.
            vertical(c, numFila, numCol);
        }else{
            pidsHijos[c] = pidAux; // Guarda los pids de los hijos para la destrucción del arbol
        }
    }
}

int main(int argc, char *argv[]){
    // ARGUMENTOS
    parse(argc, argv);
    int numFila = atoi(argv[1]);
    int numCol = atoi(argv[2]);

    pid_Padre = getpid(); // Guardamos el pid del padre
    pid_t pidsHijos[numCol]; // y declaramos un array para el de los hijos

    horizontal(numFila, numCol, pidsHijos);

    signal(SIGUSR1, despertar); // Configurar señal
    pause(); // Espera la señal que le manda el último hijo

    funcion_pstree();
    wait(NULL); // Espera a que pstree termine
    for(int i=0; i<numCol; i++){ // Manda la señal de terminación a todos los hijos que tiene
         kill(pidsHijos[i], SIGUSR1); 
    }
   

    // Espera por todos los hijos que tenga
    for (int i = 0; i < numCol; i++) { 
        wait(NULL);
    }

    return 0;
}