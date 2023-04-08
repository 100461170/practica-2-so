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


/* Timer */
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

/*Con Acc es una variable de entorno, se crea fuera de la función.*/
int valor_Acc = 0;
int mycalc(char *argv[]){
    int resultado, resto;
    // Faltaria calcular el numero de argumentos y arrojar un error si no es correcto
    if (strcmp(argv[3], "add") == 0) {
        resultado = atoi(argv[2]) + atoi(argv[4]);
        valor_Acc = valor_Acc + resultado;
        printf("a");
        return 0; }
    else if (strcmp(argv[3], "mul") == 0) {
        resultado = atoi(argv[2]) * atoi(argv[4]);
        printf("[OK] %s * %s = %d\n", argv[2], argv[4], resultado);
        return 0; }
    else if (strcmp(argv[3], "div") == 0) {
        resultado = atoi(argv[2]) / atoi(argv[4]);
        resto = atoi(argv[2]) % atoi(argv[4]);
        printf("[OK] %s / %s = %d; Resto %d\n", argv[2], argv[4], resultado, resto);
        return 0; }
    else {
        printf("[ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2>\n");
        return -1;
    }
}

void print_mytime(){
    unsigned milisegundos, segundos, minutos, horas, tiempo;
    char *horario[] = (char*)malloc(sizeof(char)*1024);
    char subcadena[21];
    /*Como mytime está en milisegundos, es necesario sacar los segundos, minutos y horas.*/
    /*Los milisegundos son el resto del paso a segundos.*/
    tiempo = mytime/1000;
    milisegundos = mytime % 1000;
    sprintf(subcadena, "%lu", milisegundos);
    horario = "." + subcadena;
    /*Los segundos son el resto del paso a minutos.*/
    tiempo = tiempo/60;
    segundos = tiempo % 60;
    sprintf(subcadena, "%u", segundos);
    if (segundos <10){
        horario = ":0" + subcadena + horario;}
    else {horario = ":" + subcadena + horario;}
    /*Los minutos son el resto del paso a horas. Las horas se dejan como tal.*/
    horas = tiempo/60;
    minutos = tiempo % 60;
    sprintf(subcadena, "%u", minutos);
    if (minutos <10){
        horario = ":0" + subcadena + horario;}
    else {horario = ":" + subcadena + horario;}
    sprintf(subcadena, "%u", horas);
    horario = horas + horario;
    printf(horario);
    free(horario);
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
        int valor, pipeid0;
        pid_t pid;
        int pipeid[2];
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
		if (command_counter > 0) {
			if (command_counter > MAX_COMMANDS){
				printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);
			}
            /* Comandos simples */
            if (command_counter == 1) {
                pid = fork();
                switch(pid) {
                    case -1:
                        perror("Error en el fork");
                        break;
                    case 0:
                        // Redirecciones
                        if (strcmp(filev[0], "0") != 0) {
                            close(0);
                            open(filev[0], O_RDONLY);
                        }
                        if (strcmp(filev[1], "0") != 0) {
                            close(1);
                            open(filev[1], O_WRONLY);
                        }
                        if (strcmp(filev[2], "0") != 0) {
                            close(2);
                            open(filev[2], O_WRONLY);
                        }
                        execvp(argvv[0][0], argvv[0]);
                        exit(-1);
                    case 1:
                        if (in_background == 0) {
                            wait(&status); }
                        break; }}
            /* Comandos con secuencias de mandatos */
            else if (command_counter > 1) {
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
                    // Redirecciono el pipe y limpio el pipe
                    if (pid == 0) {
                        // Primero redireccionamos, si es necesario, la entrada, salida y salida de errores
                        if (p == 0) {
                            if (strcmp(filev[0], "0") != 0) {
                                close(0);
                                open(filev[0], O_RDONLY);
                            }
                        }
                        if (p == command_counter - 1) {
                            if (strcmp(filev[1], "0") != 0) {
                                close(1);
                                open(filev[1], O_WRONLY);
                            }
                            if (strcmp(filev[2], "0") != 0) {
                                close(2);
                                open(filev[2], O_WRONLY);
                            }
                        }
                        if (p != 0) {
                            close(0);
                            dup(pipeid0);
                            close(pipeid0); }
                        if (p != command_counter - 1) {
                            close(1);
                            dup(pipeid[1]);
                            close(pipeid[0]);
                            close(pipeid[1]); }
                        execvp(argvv[p][0], argvv[p]);
                        perror("execvp: ");
                        exit(-1);}
                    else {
                        if (p != command_counter - 1) {
                            pipeid0 = pipeid[0];
                            close(pipeid[1]); }
                        else {
                            close(pipeid[0]); }}}
                // El padre esperará por el ultimo hijo solo si no es un proceso en background
                if (in_background == 0) {
                    while(pid != wait(&status)); }}
			else {
				// Print command
				print_command(argvv, filev, in_background);
			}


	}
	
	return 0;
}
}

