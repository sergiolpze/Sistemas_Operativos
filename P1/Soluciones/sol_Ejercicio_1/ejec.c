#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

pid_t pid_Padre;
pid_t pid_A;

void parse(int argc, char *argv[]){
    if(argc != 2){
        printf("ERROR: número de argumentos incorrectos\n");
        exit(1);
    }
}

void codigo_B(){
    for(int i=0; i<3; i++){
        switch(fork()){
            case -1:
                printf("Error al crear un hijo de B.");
                exit(1);
                break;
            case 0:
                if(i==0) codigo_X();
                if(i==1) codigo_Y();
                if(i==2) codigo_Z();
                break;
        }
    }
    for(int i=0; i<3; i++){
        wait(NULL);
    }
}

void codigo_A(){
    switch(fork()){
        case -1:
            // ERROR
            printf("Error al crear el proceso B.\n");
            exit(1);
            break;
        case 0:
            // HIJO
            printf("Soy el proceso B mi pid es %d. Mi padre es %d. Mi abuelo es %d.\n", getpid(), getppid(), pid_Padre);
            codigo_B();

            printf("Soy B (%d) y muero.\n", getpid());
            exit(0);
            break;
        default:
            // PADRE
            wait(NULL);

            break;
    }
}

int main(int argc, char *argv[]){

    parse(argc, argv);
    int segundos_alarma = atoi(argv[1]);
    
    pid_Padre = getpid();
    printf("Soy el proceso ejec: mi pid es %d\n", pid_Padre);

    switch(fork()){
        case -1: 
            // ERROR
            perror("Error al crear el proceso A\n");
            exit(1);
            break;
        case 0: 
            // HIJO
            pid_A = getpid();

            printf("Soy el proceso A: mi pid es %d. Mi padre es %d.\n", getpid(), pid_Padre);
            codigo_A();

            printf("Soy A (%d) y muero\n", getpid());
            exit(0);
            break;
        default:
            // PADRE
            wait(NULL);

            printf("Soy ejec (%d) y muero\n", getpid());
            break;
    }
    return 0;
}