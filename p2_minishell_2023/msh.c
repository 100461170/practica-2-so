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
		if (command_counter > 0) {
			if (command_counter > MAX_COMMANDS){
				printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);
			}
			else {
				// Print command
				print_command(argvv, filev, in_background);
			} 

			/* Comandos simples */
			if (command_counter == 1) {
				int pid;
				if (in_background == 0) {
					pid = fork();
					switch(pid) {
						case '-1': perror("Error en el fork");
							   	   break;
						case '0':  execvp(argvv[0][0], argvv[0]);
							       exit(-1);
						case '1':  wait(&status);
							       break;
					}
				}
				else {
					pid = fork();
					switch(pid) {
						case '-1': perror("Error en el fork");
							   	   break;
						case '0':  execvp(argvv[0][0], argvv[0]);
							       exit(-1);
					}
				}
		
			}

	}
	
	return 0;
}
}
/*Con Acc es una variable de entorno, se crea fuera de la función.*/
int valor_Acc = 0;

void mycalc(int argc, char *argv[]){
	/*Si hay menos de 5 argumentos (programa, entorno del archivo, operador1, operación y operador2), salta error.*/
	int resultado, resto;
	if (argc < 5){
		perror("La estructura del comando es mycalc %d %s %d\n", argv[2], argv[3], argv[4]);
		return -1;
	}
	switch(argv[3]){
		case 'add': resultado = argv[2] + argv[4];
					valor_Acc = valor_Acc + resultado;
					printf("[OK] %d + %d = %d, Acc %d\n", argv[2], argv[4], resultado, valor_Acc);
					break;
		case 'mul': resultado = argv[2]*argv[4];
					printf("[OK] %d * %d = %d\n", argv[2], argv[4], resultado);
					break;
		case 'div': resultado = argv[2]/argv[4];
					resto = argv[2] % argv[4];
					printf("[OK] %d/%d = %d, Resto %d\n", argv[2], argv[4], resultado), resto;
					break;
		default: perror("La estructura del comando es mycalc %d %s %d\n", argv[2], argv[3], argv[4]);
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