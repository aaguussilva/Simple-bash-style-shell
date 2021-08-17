#include "execute.h"
#include "builtin.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <gmodule.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include "tests/syscall_mock.h"
#define READ 0
#define WRITE 1

static char ** scommand_adapted(scommand self){         // Transforma el comando en array
    char **argv = calloc(scommand_length(self)+1,sizeof(char *));
    guint len = scommand_length(self);
    for (guint i = 0 ; i < len ; i++){
        argv[i] = strdup(scommand_front(self));
        scommand_pop_front(self);
    }
    argv[(len)]= NULL;
return argv;
}

static void check_fd_out(int fd){   //Chequeo de fallos para out
    if (fd == -1){
        perror("fallo open");
        exit(EXIT_FAILURE);
    }
    if(dup2(fd,1) == -1){
        perror("fallo dup");
        exit(EXIT_FAILURE);
    }
    if(close(fd) == -1){
        perror("fallo close");
        exit(EXIT_FAILURE);
    }
}

static void check_fd_in(int fd){    // Chequeo de fallos para in
    if (fd == -1){
        perror("fallo open");
        exit(EXIT_FAILURE);
    }
    if(dup2(fd,0) == -1){
        perror("fallo dup");
        exit(EXIT_FAILURE);
    }
    if(close(fd) == -1){
        perror("fallo close");
        exit(EXIT_FAILURE);
    }
}

static void ex_cmd(pipeline apipe){     //Ejecuta comando interno o externo sin pipes
    if(builtin_is_internal(pipeline_front(apipe))){
        builtin_exec(pipeline_front(apipe));
    }else{
        char ** cmd_adapted = scommand_adapted(pipeline_front(apipe));
        pid_t pid = fork();
        if (pid < 0){           // fork failed; exit
            fprintf(stderr, "fork failed\n");
            exit(EXIT_FAILURE);
        }else if ( pid > 0){    // padre
            if (pipeline_get_wait(apipe)){
                wait(NULL);
            }
        }else{     // hijo
            char *in = scommand_get_redir_in(pipeline_front(apipe));
            char *out = scommand_get_redir_out(pipeline_front(apipe));
            if ( in == NULL && out == NULL){        // No hay in ni out
                execvp(cmd_adapted[0],cmd_adapted);
            
            }else if (in == NULL && out != NULL){   // Hay solo out
                int fd = open(out, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);  // agregamos permiso de lectura
                check_fd_out(fd);
                execvp(cmd_adapted[0],cmd_adapted);
                perror("error exec");
            
            }else if(in != NULL && out == NULL){    // Hay solo in
                int fd = open(in,O_RDONLY, S_IRUSR);
                check_fd_in(fd); 
                execvp(cmd_adapted[0],cmd_adapted);
            
            }else{                                  // Hay in y out
                int fd1 = open(in, O_RDONLY ,S_IRUSR); // agregamos permiso de lectura
                check_fd_in(fd1);
                int fd = open(out, O_CREAT|O_WRONLY,S_IWUSR | S_IRUSR);
                check_fd_out(fd);
                execvp(cmd_adapted[0],cmd_adapted);                    
            }
        }   
    }
}

void execute_pipeline(pipeline apipe){
    assert(apipe != NULL);
    unsigned int N = pipeline_length(apipe);
    bool wait_p = pipeline_get_wait(apipe);
    int status;
    if(N == 1){                          // Solo un comando
        ex_cmd(apipe);
    }else if (N >= 2){                   // Hay al menos 2 comandos
        int ** fd = calloc(N , sizeof(int *));      // Pedimos memoria para una matriz de FD
        for ( guint i = 0 ; i < N ; i++){
            fd[i] = calloc(2 , sizeof(int));
        }
        pid_t pid;
        GQueue * adapted_commands = g_queue_new();
        for (unsigned int i = 0 ; i < N ; i++){     // Metemos en una pila todos los comandos del pipe
            g_queue_push_tail(adapted_commands,scommand_adapted(pipeline_front(apipe)));
            pipeline_pop_front(apipe);              // Vamos liberando el pipeline
        }
        for (unsigned int i = 0 ; i < N ; i++){     // Vamos ejecutando todos los comandos conectados con pipes
            char ** cmd = g_queue_pop_head(adapted_commands);
            if (i == 0){                            // Primer comando solo hace write sobre el pipe
                pipe(fd[i]);
                pid = fork();
                if(pid < 0){
                    fprintf(stderr, "fork failed\n");
                    exit(EXIT_FAILURE);
                }else if (pid == 0){                     // hijo
                    close(fd[i][READ]);                  // cerramos entrada de lectura del pipe
                    dup2(fd[i][WRITE],STDOUT_FILENO);    // cambiamos el out
                    close(fd[i][WRITE]);                 // cerramos la escritura del pipe
                    execvp(cmd[0],cmd);                  
                }else if (pid > 0){                      //padre
                   close(fd[i][WRITE]);
                }
            }else if (i > 0 && i != (N-1)){             //Hacen read and write sobre el pipe
                pipe(fd[i]);
                pid = fork();
                if(pid < 0){                            //fallo fork
                    fprintf(stderr, "fork failed\n");
                    exit(EXIT_FAILURE);
                }else if (pid == 0){        //hijo 
                    close(fd[i-1][WRITE]);
                    dup2(fd[i-1][READ],STDIN_FILENO);
                    close(fd[i-1][READ]);
                    close(fd[i][READ]);
                    dup2(fd[i][WRITE],STDOUT_FILENO);
                    close(fd[i][WRITE]);
                    execvp(cmd[0],cmd);
                }else if(pid > 0){          // padre
                    close(fd[i-1][READ]);
                    close(fd[i][WRITE]);
                }
            }else if(i == N-1){             // Ultimo comando solo hace read del pipe
                pid = fork();
                if ( pid < 0) {
                    fprintf(stderr, "fork failed\n");
                    exit(EXIT_FAILURE);
                  }else if (pid == 0){
                    close(fd[i-1][WRITE]);
                    dup2(fd[i-1][READ],STDIN_FILENO);
                    close(fd[i-1][READ]);
                    execvp(cmd[0],cmd);
                }else if (pid > 0){
                    close(fd[i-1][READ]);
                }
            }
        }
        for(guint i = 0; i < N && wait_p; i++){ // espera a que terminen los procesos hijos
            wait(&status);                      //La segunda condicion determina si se ejecuta o no el for
        }
        for(guint i=0; i < N; i++){             // libera los pipe llamados con calloc
            free(fd[i]);
        }
        free(fd);                               // libera el doble puntero
    }
}
