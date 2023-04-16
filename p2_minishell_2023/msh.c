//P2-SSOO-22/23

// MSH main file
// Write your msh source code here

//#include "parser.h"
#include <stddef.h>         /* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#define MAX_COMMANDS 8


// ficheros por si hay redirección
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];


void siginthandler(int param)
{
    printf("****  Saliendo del MSH **** \n");
    //signal(SIGINT, siginthandler);
    exit(0);
}


///* Timer */
pthread_t timer_thread;
unsigned long  mytime = 0;

void* timer_run ( )
{
    while (1)
    {
        usleep(1000);
        mytime++;
    }
}

/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
    //reset first
    for(int j = 0; j < 8; j++)
        argv_execvp[j] = NULL;

    int i = 0;
    for ( i = 0; argvv[num_command][i] != NULL; i++)
        argv_execvp[i] = argvv[num_command][i];
}
int time_in_shell(){
    // encontrar los tiempos de los segundos, minutos, horas
    int tiempo_restante = mytime;
    int horas = mytime/3600000;
    tiempo_restante = tiempo_restante - (horas*3600000);
    int minutos = tiempo_restante/60000;
    tiempo_restante = tiempo_restante - (minutos*60000);
    int segundos = tiempo_restante/1000;
    // convertirlos a strings
    char segundos2[10];
    char minutos2[10];
    char horas2[100];
    // si son menores que 10 hay que agregar "0" a la cadena
    if (10 > horas){
        sprintf(horas2, "0%d", horas);
    } else{
        sprintf(horas2, "%d", horas);
    }
    if (10 > minutos){
        sprintf(minutos2, "0%d", minutos);
    } else{
        sprintf(minutos2, "%d", minutos);
    }
    if (10 > segundos){
        sprintf(segundos2, "0%d", segundos);
    } else{
        sprintf(segundos2, "%d", segundos);
    }
    // imprimir el resultado en pantalla
    dprintf(2,"%s:%s:%s\n", horas2, minutos2, segundos2);
    return 0;
}
int mycalc(char *argv[]){
    int resultado, resto, nuevo_acc_int;
    char * Acc;
    char nuevo_acc_str[5];
    // Si el comando es mycalc pero alguno de los argumentos no existe, arrojamos un error
    if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL) {
        printf("[ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2>\n");
        return -1;
    }
    // Comprobamos que operación es:
    if (strcmp(argv[2], "add") == 0) {
        // Calculamos la operación
        resultado = atoi(argv[1]) + atoi(argv[3]);
        /* Si estamos en add, hay que crear una variable del entorno 'Acc', que almacenará
        la suma de resultados de las operaciones de add */
        // Si no existe ya, se crea 'Acc' con valor 0
        if (getenv("Acc") == NULL) {
            setenv("Acc", "0", 1); }
        // Cogemos la variable
        Acc = getenv("Acc");
        // Calculamos su nuevo valor y la actualizamos mediante setenv. Será necesrio que sea un char*
        nuevo_acc_int = atoi(Acc) + resultado;
        sprintf(nuevo_acc_str, "%d", nuevo_acc_int);
        setenv("Acc", nuevo_acc_str, 1);
        Acc = getenv("Acc");
        // Imprimimos el resultado
        dprintf(2, "[OK] %s + %s = %d; Acc %s\n", argv[1], argv[3], resultado, Acc);
        return 0; }
    else if (strcmp(argv[2], "mul") == 0) {
        // Calculamos el resultado y lo imprimimos
        resultado = atoi(argv[1]) * atoi(argv[3]);
        dprintf(2, "[OK] %s * %s = %d\n", argv[1], argv[3], resultado);
        return 0; }
    else if (strcmp(argv[2], "div") == 0) {
        // Calculamos el resultado y el resto y los imprimimos
        resultado = atoi(argv[1]) / atoi(argv[3]);
        resto = atoi(argv[1]) % atoi(argv[3]);
        dprintf(2, "[OK] %s / %s = %d; Resto %d\n", argv[1], argv[3], resultado, resto);
        return 0; }
    else {
        // Si la operacion no era add/mul/div imprimimos el mensaje de error
        printf("[ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2>\n");
        return -1;
    }
}

/**
 * Main sheell  Loop
 */
int main(int argc, char* argv[])
{
    /**** Do not delete this code.****/
    int end = 0;
    int executed_cmd_lines = -1;
    char *cmd_line = NULL;
    char *cmd_lines[10];

    if (!isatty(STDIN_FILENO)) {
        cmd_line = (char*)malloc(100);
        while (scanf(" %[^\n]", cmd_line) != EOF){
            if(strlen(cmd_line) <= 0) return 0;
            cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1);
            strcpy(cmd_lines[end], cmd_line);
            end++;
            fflush (stdin);
            fflush(stdout);
        }
    }

    pthread_create(&timer_thread,NULL,timer_run, NULL);

    /*********************************/

    char ***argvv = NULL;
    int num_commands;


    while (1)
    {
        int status = 0;
        int command_counter = 0;
        int in_background = 0;
        signal(SIGINT, siginthandler);
        // Prompt
        write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

        // Get command
        //********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
        executed_cmd_lines++;
        if( end != 0 && executed_cmd_lines < end) {
            command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
        }
        else if( end != 0 && executed_cmd_lines == end){
            return 0;
        }
        else{
            command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
        }
        //************************************************************************************************

        /************************ STUDENTS CODE ********************************/
        int pipeid[2];
        pid_t pid, pid_b;
        int valor, pipeid0;
        if (command_counter > 0) {
            if (command_counter > MAX_COMMANDS) {
                printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);
            }
            // Comandos simples
            if (command_counter == 1) {
                // Hay 2 tipos de comandos simples: los mandatos internos y los normales
                // Comprobamos primero si se ha llamado a un mandato interno
                if (strcmp(argvv[0][0], "mycalc") == 0) {
                    mycalc(argvv[0]);
                }
                else if (strcmp(argvv[0][0], "mytime") == 0) {
                    time_in_shell();
                }
                // Si no es un mandato interno, será un mandato normal
                else {
                    // Creamos el proceso hijo
                    pid = fork();
                    switch(pid) {
                        case -1:
                            perror("Error en el fork");
                            break;
                        case 0:
                            // Redirecciones de fichero en caso de que sean necesarias
                            if (strcmp(filev[0], "0") != 0) {
                                close(0);
                                open(filev[0], O_RDONLY | O_CREAT, 0644);
                            }
                            if (strcmp(filev[1], "0") != 0) {
                                close(1);
                                open(filev[1], O_WRONLY | O_CREAT, 0644);
                            }
                            if (strcmp(filev[2], "0") != 0) {
                                close(2);
                                open(filev[2], O_WRONLY | O_CREAT, 0644);
                            }
                            // Cambiamos la imagen del proceso por la pedida en el mandato
                            execvp(argvv[0][0], argvv[0]);
                            exit(-1);
                        default:
                            // Si el proceso no es en background, el padre esperará al proceso hijo
                            if (in_background == 0) {
                                // Como puede haber procesos en background vamos a esperar hasta que termine este hijo
                                // concreto
                                while(pid != wait(&status));
                                // Esperamos por el ultimo proceso en background antes del nuevo mandato en foreground,
                                // para que no se quede zombie
                                if (pid_b > 0) {
                                    waitpid(pid_b, &status, 0);
                                    pid_b = 0;
                                }
                                break; }
                            else {
                                pid_b = pid;
                                break;
                            }}}}
            // Comandos con secuencias de mandatos
            else if (command_counter > 1) {
                // La creacion de las pipes es de forma genérica, para n procesos
                for (int p=0; p < command_counter; p++) {
                    // Si no es el ultimo hijo creo el pipe
                    if (p != command_counter - 1) {
                        valor = pipe(pipeid);
                        if (valor < 0) {
                            perror("Error al crear la pipe: ");
                            exit(-1); }}
                    // Creo el hijo
                    pid = fork();
                    if (pid < 0) {
                        perror("Error en la creacion del hijo: ");
                        exit(-1); }
                    // Redirección y limpieza de la pipe
                    // Procesos hijo
                    if (pid == 0) {
                        /* Primero redireccionamos, si es necesario, la entrada, salida y salida de errores.
                        En el caso de la redireccion de entrada, se hace en el primer proceso, mientras
                        que las de salida se hacen en el último */
                        if (p == 0) {
                            if (strcmp(filev[0], "0") != 0) {
                                close(0);
                                open(filev[0], O_RDONLY | O_CREAT, 0644);
                            }
                        }
                        if (p == command_counter - 1) {
                            if (strcmp(filev[1], "0") != 0) {
                                close(1);
                                open(filev[1], O_WRONLY | O_CREAT, 0644);
                            }
                            if (strcmp(filev[2], "0") != 0) {
                                close(2);
                                open(filev[2], O_WRONLY | O_CREAT, 0644);
                            }
                        }
                        // Si no es el primer proceso cambiamos la entrada estándar a la del pipe
                        if (p != 0) {
                            close(0);
                            dup(pipeid0);
                            close(pipeid0); }
                        // Si no es el último proceso cambiamos la salida estándar a la del pipe
                        if (p != command_counter - 1) {
                            close(1);
                            dup(pipeid[1]);
                            close(pipeid[0]);
                            close(pipeid[1]); }
                        // Una vez redireccionada la pipe, cambiamos la imagen del proceso por la del mandato
                        execvp(argvv[p][0], argvv[p]);
                        perror("execvp: ");
                        exit(-1);}
                    // Proceso padre
                    else {
                        // Si no estamos en el ultimo proceso clonamos la entrada del pipe para poder reutilizarla
                        if (p != command_counter - 1) {
                            pipeid0 = pipeid[0];
                            close(pipeid[1]); }
                        // Si es el ultimo cerramos la entrada, cerrando la pipe definitivamente
                        else {
                            close(pipeid[0]); }}}
                // El padre esperará por el ultimo hijo si no es un proceso en background
                if (in_background == 0) {
                    while(pid != wait(&status));
                    // Esperamos por el ultimo proceso en background antes del nuevo mandato, para que no se quede zombie
                    if (pid_b > 0) {
                        waitpid(pid_b, &status, 0);
                        pid_b = 0;
                    }}
                else {
                    pid_b = pid;
                }}
        }

    }
    return 0;
}