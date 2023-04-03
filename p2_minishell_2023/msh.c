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
#include <signal.h>

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

	/*Imprimir mandatos de argvv con un bucle for*/
	longitud = sizeof(argvv);
	for (i=0, i<longitud, i++){
		printf("Mandato numero %d = %s/n", i, argvv[i][0]);
		num_mandatos++;
		if (argvv[i][1] != NULL){
			printf("Argumento %d = %s/n", i argvv[i][1]);
		}
	}
	return num_mandatos;
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


/**
 * Main sheell  Loop  
 */
int main(int argc, char* argv[])
{
	/**** Do not delete this code.****/
	int end = 0; 
	int pid, espera;
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
		if (command_counter > 0) {
			if (command_counter > MAX_COMMANDS){
				printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);
			}
			else {
				// Print command
				print_command(argvv, filev, in_background);
			} pid = fork();
			/*Si está entre 0 y 8, entonces podemos ejecutar los comandos desde el hijo*/
			switch(pid){
				case '-1': perror("Error en el fork");
						   break;
				case '0': printf("El hijo");
						  execvp(argvv[0][0], argvv[0][0], argvv[0][1], NULL);
						  exit(-1);
				case '1': printf("El padre");
						  wait(&espera);
						  break;
			}
		
		}
		/* Comandos simples */
		if (command_counter == 1) {
			int pid;
			if (background == 0) {
				pid = fork();
				if (pid == 0) {
					execvp(argvv[0][0], argvv[0]);
				}
				else {
					wait(&status);
				}
			}
			else {
				pid = fork();
				if (pid == 0) {
					execvp(argvv[0][0], argvv[0]);
				}
			}
		}


		}
	}
	
	return 0;
}

void controlC(int s){
	printf("The signal worked. Time: %d/n", s);
	return 0;
}


int read_command(char ***argvv, char **filev, int *bg){
	int fd, num_mandatos = 0, longitud, i;
	struct signation cero;

	/*Cuando se pulsa control C*/
	cero.sa_handler = controlC;
	cero.sa_flags = 0;
	sigemptyset(&(cero.sa_mark));
	signation(SIGINT, &cero, NULL);

	/*Posicion del fichero*/
	if (filev[0] != NULL){
		printf("Fichero %s con redireccionamiento de entrada(<).", file[0]);
	}
	else if (filev[1] != NULL){
		printf("Fichero %s con redireccionamiento de salida(>).", file[1]);
	}
	else if (filev[2]!= NULL){
		printf("Fichero %s con redireccionamiento de salida de error(!>).", file[2]);
	}
	else {
		printf("Ningun fichero se usa como redireccion.");
	}

	/*Proceso background*/
	if (bg == 0){
		printf("Proceso not background.");
	}
	else {
		printf("Proceso en background");
	}