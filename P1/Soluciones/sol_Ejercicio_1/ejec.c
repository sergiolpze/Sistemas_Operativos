#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void parse(int argc, char *argv[]){
    if(argc != 2){
        printf("ERROR: número de argumentos incorrectos\n");
        exit(1);
    }
}

int main(int argc, char *argv[]){

    parse(argc, argv);
    int segundos_alarma = atoi(argv[1]);
    
    pid_t pid_Padre = getpid();
    printf("Soy el proceso ejec: mi pid es %d\n", pid_Padre);


    pid_t pid_A = fork();

    switch(pid_A){
        case -1: 
            perror("Error al crear el proceso A");
            exit(1);
            break;
        case 0: 
            printf("Soy el proceso A: mi pid es %d. Mi padre es %d.", pid_A, pid_Padre);
            break;
        default:
            
            break;
    }
    return 0;
}