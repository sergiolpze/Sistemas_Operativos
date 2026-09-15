#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    pid_t pid = getpid();

   printf ("Soy el proceso ejec: mi pid es %d\n", pid);

   pid = fork();

   return 0;
}