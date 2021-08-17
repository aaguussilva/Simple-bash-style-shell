#include "builtin.h"        // Cabecera 
#include <assert.h>         // Para tener assert
#include <unistd.h>         // Para usar la syscall chdir
#include <stdlib.h>         // Para tener la libreria standar
#include <string.h>         // Para usar funciones de cadena de caracteres
#include "tests/syscall_mock.h" // chequeo del test
#include <sys/types.h>  //wait
#include <sys/wait.h>   // wait
#include <stdio.h>

bool builtin_is_internal(scommand cmd){         // Comparamos si el primer comando es cd o exit
    assert(cmd != NULL);
    int a = strcmp(scommand_front(cmd),"cd");       
    int b = strcmp(scommand_front(cmd),"exit"); 
    return (a == 0 || b == 0);
}

void builtin_exec(scommand cmd){
    assert(builtin_is_internal(cmd));
    if (strcmp(scommand_front(cmd),"cd") == 0){     // Comparamos si el comando es cd
        scommand_pop_front(cmd);                    // Sacamos el primer elemento
        chdir(scommand_front(cmd));                 // Ejecutamos la syscall
    }else{                                         // El comando es exit
        if(waitpid(-1,0,WNOHANG)==0){               // el mata zombies
            pid_t pid = getppid();
            printf("mi pid es %d\n", pid);
            kill(pid,SIGKILL);
        }
        exit(EXIT_SUCCESS);                         // Usamos funcion exit 
    }
}


