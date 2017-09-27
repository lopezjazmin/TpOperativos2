/*
 ============================================================================
 Name        : Worker.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Worker.h"
#include "socketConfig.h"

#define MAX_LINE 4096

unsigned long int lineCountFile(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    unsigned long int linecount = 0;
    int c;
    if(fp == NULL){
    	printf("No se puede abrir el archivo.\n");
        fclose(fp);
        return 0;
    }
    while((c=fgetc(fp)) != EOF )
    {
        if(c == '\n')
            linecount++;
    }
    fclose(fp);
    return linecount;
}

void sortfile(char **array, int linecount)
{
    int i, j;
    char t[MAX_LINE];

    for(i=1;i<linecount;i++)
    {
        for(j=1;j<linecount;j++)
        {
            if(strcmp(array[j-1], array[j]) > 0)
            {
                strcpy(t, array[j-1]);
                strcpy(array[j-1], array[j]);
                strcpy(array[j], t);
            }
        }
    }
}


char* apareo (char* paths []){
	int cantPaths = sizeof(paths) / sizeof(paths[0]) + 1;

	int i = 0;
	unsigned long int linecountGlobal;

	for(i=0;i<cantPaths;i++){
		linecountGlobal = linecountGlobal + lineCountFile(paths[i]) + 1;
	}
	char **arrayGlobal = (char**)malloc(linecountGlobal * sizeof(char*));

	for(i=0; i<cantPaths; i++){
		FILE *fileIN;
		fileIN = fopen(paths[i], "rb");
		if(!fileIN)
		{
			printf("No se puede abrir el archivo.\n");
			exit(-1);
		}

		unsigned long int linecount = lineCountFile(paths[i]);
		linecount += 1;

		char **array = (char**)malloc(linecount * sizeof(char*));
		char singleline[MAX_LINE];

		int i = 0;
		while(fgets(singleline, MAX_LINE, fileIN) != NULL)
		{
			array[i] = (char*) malloc (MAX_LINE * sizeof(char));
			singleline[MAX_LINE] = '\0';
			strcpy(array[i], singleline);
			i++;
		}
		strcat(array[linecount - 1], "\n");

		if(arrayGlobal[0] == NULL){
			memcpy(arrayGlobal, array, linecount * sizeof(char*));
		}else{
			memcpy(arrayGlobal+linecount, array, linecount * sizeof(char*));
		}

		fclose(fileIN);
	}

	sortfile(arrayGlobal, linecountGlobal);

	FILE *archivoOrdenado = fopen("fileTestOrdenado", "wb");
	if(!archivoOrdenado)
		{
			printf("No se puede abrir el archivo.\n");
			exit(-1);
		}

	for(i=0; i<linecountGlobal; i++)
	{
		fprintf(archivoOrdenado,"%s", arrayGlobal[i]);
	}

	fclose(archivoOrdenado);

	for(i=0; i<linecountGlobal; i++)
	{
		free(arrayGlobal[i]);
	}

	return "fileTestOrdenado";
}

char* ordenarArchivo(char* path){
	char *out = "fileTestOrdenado";

	FILE *fileIN, *fileOUT;

	fileIN = fopen(path, "rb");
	if(!fileIN)
	{
		exit(-1);
	}

	unsigned long int linecount = lineCountFile(path);
	linecount += 1;

	char **array = (char**)malloc(linecount * sizeof(char*));
	char singleline[MAX_LINE];

	int i = 0;
	while(fgets(singleline, MAX_LINE, fileIN) != NULL)
	{
		array[i] = (char*) malloc (MAX_LINE * sizeof(char));
		singleline[MAX_LINE] = '\0';
		strcpy(array[i], singleline);
		i++;
	}

	sortfile(array, linecount);

	for(i=0; i<linecount; i++)
	{
		printf("%s\n", array[i]);
	}

	fileOUT = fopen(out, "wb");
	if(!fileOUT)
	{
		exit(-1);
	}

	for(i=0; i<linecount; i++)
	{
		fprintf(fileOUT, "%s", array[i]);
	}

	fclose(fileIN);
	fclose(fileOUT);

	for(i=0; i<linecount; i++)
	{
		free(array[i]);
	}
	free(array);

	return 0;
}


int main(void) {
	t_log* logger;
	char* fileLog;
	fileLog = "WorkerLogs.txt";

	char* paths [2];
	paths[0] = "fileTest";
	paths[1] = "fileTest2";

	//char* pathFinal = ordenarArchivo("fileTest");
	char* pathFinal = apareo(paths);

	printf("Inicializando proceso Worker\n");
	logger = log_create(fileLog, "Worker Logs", 0, 0);
	log_trace(logger, "Inicializando proceso Worker");

	worker_configuracion configuracion = get_configuracion();
	log_trace(logger, "Archivo de configuracion levantado");

	un_socket socketServer=socket_escucha("127.0.0.1", configuracion.PUERTO_WORKER);
	listen(socketServer, 999);
	while(1){
		un_socket socketConexion=aceptar_conexion(socketServer);
		if(socketConexion >0)
		{
			int pid=fork();

					switch(pid)
					{
						case -1: // Si pid es -1 quiere decir que es el proceso padre, ha habido un error
							perror("No se ha podido crear el proceso hijo\n");
							break;
						case 0: // Cuando pid es cero quiere decir que es el proceso hijo
						{

							t_paquete* paquete_recibido = recibir(socketConexion);
							esperar_handshake(socketConexion, paquete_recibido, cop_handshake_master);

							switch(paquete_recibido->codigo_operacion){ //revisar validaciones de habilitados
								case cop_handshake_master:

								break;
								case cop_handshake_worker:

								break;
								case cop_worker_tranformacion:
										//procesar tr paquete_recibido->data
								break;
								case cop_worker_reduccionLocal:

								break;
								case cop_worker_reduccionGlobal:

								break;
							}

						}
					break;
				}
		}
	}
	return EXIT_SUCCESS;
}





/*
 *

 * */
