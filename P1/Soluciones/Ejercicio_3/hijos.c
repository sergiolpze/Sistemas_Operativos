#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int shmid; // id del segmento
int *hijosFinales; // Array compartido

// ANALISIS ARGUMENTOS
void parse(int argc, char *argv[]){
    if(argc != 3){
        printf("ERROR: número de argumentos incorrectos.\n");
        exit(1);
    }if(atoi(argv[1])<=0 || atoi(argv[2])<=0){
        printf("ERROR: las dimensiones del arbol deben ser mayores a 0.\n");
        exit(1);
    }
}

void creaComp(int y){
    // Crear segmento
    shmid = shmget(IPC_PRIVATE, y * sizeof(int), IPC_CREAT | 0666); // Creamos el segmento compartido de tamaño 
    if(shmid == -1){                                                // enteros * cantidad de hijos finales en bytes
        perror("Error al crear la memoria compartida");
        exit(1);
    }

    // Vincular segmento al proceso
    hijosFinales = (int *) shmat(shmid, 0, 0); 
    if(hijosFinales == (int *) -1){
        perror("Error al vincular la memoria compartida");
        exit(1);
    }
}

// Cada hijo final se añade al array y hace su print.
void codigoHijosFinales(int x, int i, const int pidsColumna[]){
    hijosFinales[i] = getpid();

    printf("Soy el subhijo %d, mis padres son: ", getpid());

    for(int j = 0; j < x; j++){
        printf("%d", pidsColumna[j]);

        if(j < x - 1){ // Si no es el último ponemos coma
            printf(", ");
        }
    }
    printf("\n"); // y el salto de línea
}

void crearUltimaFila(int x, int y, const int pidsColumna[]){
    pid_t pid;

    for(int i = 0; i < y; i++){
        pid = fork();
        switch(pid){
            case -1:
                // ERROR
                perror("Error en fork");
                exit(1);
            case 0:
                // HIJO 
                codigoHijosFinales(x, i, pidsColumna);
                exit(0);
        }
    }
}

void crearProcesos(int x, int y){
    int pidsColumna[x];          // pids de los x procesos de la columna
    pid_t pid;

    // Columna de x procesos
    for(int i = 0; i < x; i++){
        pid = fork();

        switch (pid){
            case -1:
                // ERROR
                perror("Error en fork");
                exit(1);
            case 0: 
                // HIJO
                pidsColumna[i] = getpid(); // El hijo se apunta y sigue

                break;
            default:
                // PADRE
                wait(NULL); // Todos los padres esperan

                if(i == 0){ // Si es el superpadre vuelve al main para hacer el print.
                    return;
                }

                exit(0); // Y los padres intermedios sí terminan
        }
    }

    // El último proceso de la columna crea los hijos finales
    crearUltimaFila(x, y, pidsColumna);

    for(int i = 0; i < y; i++) // El último proceso de la columna espera a todos sus hijos
        wait(NULL);
    exit(0);
}

void printSuperPadre(int y){
    printf("Soy el superpadre (%d) : mis hijos finales son: ", getpid());
    // imprimimos el pid de todos los hijos finales
    for(int i = 0; i < y; i++){ 
        printf("%d", hijosFinales[i]);
        if(i < y - 1){          // si no es el último, ponemos coma
            printf(", ");
        }
    }
    printf("\n"); 
}

void borraComp(void){
    // Desvincular segmento
    if(shmdt((char *) hijosFinales) < 0){
        perror("Error al desvincular la memoria compartida");
        exit(1);
    }

    // Eliminar segmento
    if(shmctl(shmid, IPC_RMID, 0) < 0){
        perror("Error al borrar la memoria compartida");
        exit(1);
    }

    hijosFinales = NULL;
}

int main(int argc, char *argv[]){
    
    parse(argc, argv);
    int x = atoi(argv[1]);
    int y = atoi(argv[2]);

    creaComp(y); // Crea el segmento de memoria compartida y lo vincula con el proceso

    crearProcesos(x, y); // Crea el arbol y hace los prints
    printSuperPadre(y);

    borraComp(); // Desvincula el segmento del proceso y lo elimina

    return 0;
}