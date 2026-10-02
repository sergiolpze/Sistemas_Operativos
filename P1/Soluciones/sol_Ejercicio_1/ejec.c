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


void manejador_alarma(int sig){}

void funcion_pstree(){
    if(fork()==0){
        // HIJO
        char strPidP[10];
        sprintf(strPidP, "%d", pid_Padre);
        execlp("pstree","pstree","-c", strPidP ,NULL);

        exit(0);
    }
}



void codigo_Z(int segundos){
    printf("Soy el proceso Z mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d.\n", getpid(), getppid(), pid_A, pid_Padre);
    
    signal(SIGALRM, manejador_alarma);
    alarm(segundos);
    pause();

    kill(pid_A, SIGUSR1);

    signal(SIGUSR1, manejador_alarma);
    pause();

    printf("Soy Z (%d) y muero\n", getpid());
    exit(0);
}

void codigo_Y(){
    printf("Soy el proceso Y mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d.\n", getpid(), getppid(), pid_A, pid_Padre);
    signal(SIGUSR1, manejador_alarma);
    pause();

    printf("Soy Y (%d) y muero\n", getpid());
    exit(0);
}

void codigo_X(){
    printf("Soy el proceso X mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d.\n", getpid(), getppid(), pid_A, pid_Padre);
    signal(SIGUSR1, manejador_alarma);
    pause();

    printf("Soy X (%d) y muero\n", getpid());
    exit(0);
}

void codigo_B(int segundos){
    printf("Soy el proceso B mi pid es %d. Mi padre es %d. Mi abuelo es %d.\n", getpid(), pid_A, pid_Padre);

    for(int i=0; i<3; i++){
        switch(fork()){
            case -1:
                printf("Error al crear un hijo de B.");

                exit(1);
            case 0:
                if(i==0) codigo_X();
                if(i==1) codigo_Y();
                if(i==2) codigo_Z(segundos);
        }
    }

    signal(SIGUSR1, manejador_alarma);

    for(int i=0; i<3; i++){
        wait(NULL);
    }
    printf("Soy B (%d) y muero.\n", getpid());

    exit(0);
}

void codigo_A(int segundos){
    printf("Soy el proceso A: mi pid es %d. Mi padre es %d.\n", getpid(), pid_Padre);

    switch(fork()){
        case -1:
            // ERROR
            printf("Error al crear el proceso B.\n");

            exit(1);
        case 0:
            // HIJO
            codigo_B(segundos);

        default:
            // PADRE
            signal(SIGUSR1, manejador_alarma);
            pause();

            funcion_pstree();
            wait(NULL);

            kill(pid_Padre, SIGUSR1); // Señal que le manda al superpadre

            wait(NULL);
            printf("Soy A (%d) y muero\n", getpid());
            exit(0);
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
        case 0: 
            // HIJO
            pid_A = getpid();
            codigo_A(segundos_alarma);

        default:
            // PADRE
            signal(SIGUSR1, manejador_alarma);
            pause();

            kill(0,SIGUSR1);

            wait(NULL);

            printf("Soy ejec (%d) y muero\n", getpid());
            exit(0);
    }
    return 0;
}