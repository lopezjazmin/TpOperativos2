/*
 ============================================================================
 Name        : Yama.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style

 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "Yama.h"

t_list* tabla_estados;
yama_configuracion configuracion;
int main(void) {
	t_log* logger;
	char* fileLog;
	fileLog = "YamaLogs.txt";
	tabla_estados= list_create();
	printf("Inicializando proceso Yama\n");
	logger = log_create(fileLog, "Yama Logs", 0, 0);
	log_trace(logger, "Inicializando proceso Yama");

    configuracion = get_configuracion();
	log_trace(logger, "Archivo de configuracion levantado");

	t_tabla_planificacion tabla;
	tabla.workers = list_create();
	t_job job;
	job.master = 1;
	job.nodo = 1;
	job.bloque = 8;
	job.etapa = transformacion;
	job.cantidadTemporal = 5;
	job.temporal = "temp1";
	job.estado = enProceso;
	procesar_job(1, job);
	job.nodo = 3;
	job.bloque = 2;
	job.temporal = "temp2";
	procesar_job(1, job);
	job.nodo = 2;
	job.bloque = 9;
	job.temporal = "temp3";
	procesar_job(1, job);
	job.master = 3;
	job.nodo = 4;
	job.bloque = 11;
	job.etapa = transformacion;
	job.temporal = "temp4";
	job.estado = enProceso;
	procesar_job(2, job);
	job.nodo = 5;
	job.bloque = 10;
	job.etapa = reduccionLocal;
	job.temporal = "temp5";
	procesar_job(2, job);
	job.nodo = 6;
	job.bloque = 14;
	job.etapa = reduccionGlobal;
	job.temporal = "temp6";
	job.estado = finalizado;
	procesar_job(2, job);
	job.nodo = 7;
	job.bloque = 15;
	job.etapa = reduccionLocal;
	job.temporal = "temp7";
	job.estado = error;
	procesar_job(2, job);
	job.nodo = 8;
	job.bloque = 12;
	job.etapa = transformacion;
	job.temporal = "temp8";
	job.estado = enProceso;
	procesar_job(2, job);

	t_job job2;
	job2.master = 1;
	job2.nodo = 1;
	job2.bloque = 8;
	job2.etapa = reduccionLocal;
	job2.cantidadTemporal = 6;
	job2.temporal = "temp20";
	job2.estado = enProceso;
	procesar_job(1, job2);

	t_job job3;
	job3.master = 3;
	job3.nodo = 4;
	job3.bloque = 11;
	job3.etapa = reduccionLocal;
	job3.cantidadTemporal = 6;
	job3.temporal = "temp30";
	job3.estado = error;
	procesar_job(2, job3);


	fd_set master;    // master file descriptor list
	fd_set read_fds;  // temp file descriptor list for select()
	int fd_max;        // maximum file descriptor number

	int listener;     // listening socket descriptor
	int newfd;        // newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	socklen_t addrlen;

	char buf[256];    // buffer for client data
	int nbytes;
	char remoteIP[INET6_ADDRSTRLEN];
	int yes=1;        // for setsockopt() SO_REUSEADDR, below
	int i, j, rv;
	struct addrinfo hints, *ai, *p;

	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, "9999", &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	for(p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) {
			continue;
		}
		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}
			break;
		}
		// if we got here, it means we didn't get bound
		if (p == NULL) {
			fprintf(stderr, "selectserver: failed to bind\n");
			exit(2);
		}
		freeaddrinfo(ai); // all done with this
		// listen
		if (listen(listener, 10) == -1) {
			perror("listen");
			exit(3);
		}
		// add the listener to the master set
		FD_SET(listener, &master);
		// keep track of the biggest file descriptor
		fd_max = listener; // so far, it's this one

	 	//Iniciar hilo consola
		pthread_t hiloFileSystem;
		//pthread_create(&hiloFileSystem, NULL, hiloFileSystem_Consola);

		//Se conecta al FileSystem
		un_socket fileSystemSocket = conectar_a(configuracion.IP_FS,configuracion.PUERTO_FS);
		FD_SET(fileSystemSocket, &master);
		if(fileSystemSocket > listener){
			fd_max = fileSystemSocket;
		}else{
			fd_max = listener;
		}
		realizar_handshake(fileSystemSocket, cop_handshake_yama);
		//Falta pedir la info de los workers conectados todo mati e aca hay que hacer un recibir
		//Que pasa si le rechazan la conexion.
		int socketActual;
		//CONEXIONES
		while(1){
			if (signal(SIGUSR1, sig_handler) == SIG_ERR){};
			read_fds = master;
			select(fd_max+1, &read_fds, NULL, NULL, NULL);
			for(socketActual = 0; socketActual <= fd_max; socketActual++) {
					if (FD_ISSET(socketActual, &read_fds)) {
						if (socketActual == listener) { //es una conexion nueva
							newfd = aceptar_conexion(socketActual);
							t_paquete* handshake = recibir(socketActual);
							FD_SET(newfd, &master); //Agregar al master SET
							if (newfd > fd_max) {    //Update el Maximo
								fd_max = newfd;
							}
							log_trace(logger, "Yama recibio una nueva conexion");
							free(handshake);
						//No es una nueva conexion -> Recibo el paquete
						} else {
							t_paquete* paqueteRecibido = recibir(socketActual);
							switch(paqueteRecibido->codigo_operacion){ //revisar validaciones de habilitados
							case cop_handshake_master:
								esperar_handshake(socketActual, paqueteRecibido, cop_handshake_master);
							break;
							case cop_archivo_programa:
								enviar(fileSystemSocket, cop_archivo_programa,paqueteRecibido->tamanio ,paqueteRecibido->data);
								//recibir un archivo
							break;
							case cop_yama_info_fs:
							{
								//Deserializacion
								t_archivoxnodo* archivoNodo=malloc(sizeof(t_archivoxnodo));
								archivoNodo->bloquesRelativos =  list_create();
								archivoNodo->nodos =  list_create();

								int longitudNombre = 0;
								int desplazamiento = 0;
								memcpy(&longitudNombre, paqueteRecibido->data + desplazamiento, sizeof(int));
								desplazamiento+=sizeof(int);
								archivoNodo->pathArchivo= malloc(longitudNombre);

								memcpy(archivoNodo->pathArchivo, paqueteRecibido->data + desplazamiento, longitudNombre);
								desplazamiento+=longitudNombre;

								//Lista bloques relativos
								int cantidadElementosBloques = 0;
								memcpy(&cantidadElementosBloques ,paqueteRecibido->data + desplazamiento,sizeof(int));
								desplazamiento+= sizeof(int);

								for(i=0;i<cantidadElementosBloques;i++){
									int bloque = 0;
									memcpy(&bloque, paqueteRecibido->data + desplazamiento, sizeof(int));
									desplazamiento+= sizeof(int);
									list_add(archivoNodo->bloquesRelativos, bloque);
								}

								//Lista nodos (t_nodoxbloques)
								int cantidadElementosNodos = 0;
								memcpy(&cantidadElementosNodos ,paqueteRecibido->data + desplazamiento,sizeof(int));
								desplazamiento+= sizeof(int);

								for(i=0;i<cantidadElementosNodos;i++){
									t_nodoxbloques* nodoBloques = malloc(sizeof(t_nodoxbloques));
									int longitudNombreNodo = 0;
									memcpy(&longitudNombreNodo, paqueteRecibido->data + desplazamiento, sizeof(int));
									desplazamiento+=sizeof(int);

									char* idNodo;
									memcpy(&idNodo ,paqueteRecibido->data + desplazamiento,sizeof(int));
									desplazamiento+= sizeof(int);
									nodoBloques->idNodo = idNodo;

									//cantidad elementos lista bloques (t_infobloque)
									int cantidadElementos = 0;
									memcpy(&cantidadElementos ,paqueteRecibido->data + desplazamiento,sizeof(int));
									desplazamiento+= sizeof(int);

									for(i=0;i<cantidadElementos;i++){
										t_infobloque* infoBloque = malloc(sizeof(t_infobloque));
										int bloqueAbsoluto = 0;
										memcpy(&bloqueAbsoluto, paqueteRecibido->data + desplazamiento, sizeof(int));
										desplazamiento+=sizeof(int);
										infoBloque->bloqueAbsoluto = bloqueAbsoluto;

										int bloqueRelativo = 0;
										memcpy(&bloqueRelativo, paqueteRecibido->data + desplazamiento, sizeof(int));
										desplazamiento+=sizeof(int);
										infoBloque->bloqueRelativo = bloqueRelativo;

										int finBloque = 0;
										memcpy(&finBloque, paqueteRecibido->data + desplazamiento, sizeof(int));
										desplazamiento+=sizeof(int);
										infoBloque->finBloque = finBloque;

										list_add(nodoBloques->bloques, infoBloque);
									}

									archivoNodo->nodos = nodoBloques;
								}

								//Evalua y planifica en base al archivo que tiene que transaformar
								void planificarBloques(void* bloque){
									int* nroBloque = (int*)bloque;
									planificarBloque(tabla , *nroBloque, archivoNodo );
								}

								list_iterate(archivoNodo->bloquesRelativos, planificarBloques);

								//Devuelve lista con los workers
								//Ahora lo debe sacar de archivoNodo workersAsignados
								char* listaWorkers;
								listaWorkers = "127.0.0.1|3000,127.0.0.1|3001,127.0.0.1|3002";
								enviar(socketActual,cop_yama_lista_de_workers,sizeof(char*)*strlen(listaWorkers),listaWorkers);

							}
								break;
							case cop_master_archivo_a_transformar:
							{
								log_trace(logger, "Recibi nuevo pedido de transformacion de un Master sobre X archivo");
								//Debe pedir al FS la composicion de bloques del archivo (por nodo)
								char* pathArchivo=(char*)paqueteRecibido->data;
								enviar(fileSystemSocket,cop_yama_info_fs,sizeof(char*)*strlen(pathArchivo),pathArchivo);


								break;
							}
							case cop_master_estados_workers:
								log_trace(logger, "Recibi estado de conexion de worker para proceso X");
								//hacer lo que corresponda
								//si esta todo ok avanza el proceso de forma normal y sino debe replanificar
								//y mandar nuevos sockets

								break;
							case cop_datanode_info :
								{
									int desplazamiento=0;
									int cantidadElementos;
									memcpy(&cantidadElementos, paqueteRecibido->data, sizeof(int));
									desplazamiento += sizeof(int);

									int i=0;
									for(;i<cantidadElementos;i++){
										t_clock* worker = malloc(sizeof(t_clock*));
										int longitudIp;
										memcpy(&longitudIp, paqueteRecibido->data + desplazamiento, sizeof(int));
										desplazamiento += sizeof(int);
										worker->ip =  malloc(longitudIp);
										memcpy(worker->ip, paqueteRecibido->data + desplazamiento, longitudIp);
										desplazamiento += longitudIp;

										memcpy(worker->puerto, paqueteRecibido->data + desplazamiento, sizeof(int));
										desplazamiento += sizeof(int);
										desplazamiento += sizeof(int);//me vuelvo a desplazar por el tamanio ya que lo ignoro

										int longitudNombre;
										memcpy(&longitudNombre, paqueteRecibido->data + desplazamiento, sizeof(int));
										desplazamiento += sizeof(int);
										worker->worker_id =  malloc(longitudNombre);
										memcpy(worker->worker_id, paqueteRecibido->data + desplazamiento, longitudNombre);
										desplazamiento += longitudNombre;

										list_add(tabla.workers,worker);
									}
									tabla.clock_actual = (t_clock*)tabla.workers->head;
								}
								break;
								}
						}
					}
			}
		}
	return EXIT_SUCCESS;
}


void sig_handler(int signo){
    if (signo == SIGUSR1){
        printf("Se recibio SIGUSR1\n");
    	//log_trace(logger, "Se recibio SIGUSR1");
    	yama_configuracion configuracion = get_configuracion();
    	printf("Se cargo nuevamente el archivo de configuracion\n");
    	//log_trace(logger, "Se cargo nuevamente el archivo de configuracion");
    }
}

void procesar_job(int jobId, t_job datos){
	t_estados* registro=(t_estados*) buscar_por_jobid(jobId);
	if(registro == NULL){
		t_estados* job= malloc(sizeof(t_estados));
		job->job=jobId;
		job->contenido = list_create();
		t_job* nuevoJob= crearJob(datos);
		list_add(job->contenido, nuevoJob);
		list_add(tabla_estados, job);
	}else{
		t_job* nodo = buscar_por_nodo(datos.nodo, registro->contenido);
		if (nodo == NULL){
			t_job* nuevoJob= crearJob(datos);
			list_add(registro->contenido, nuevoJob);
		}else{
			setearJob(nodo, datos);
		}
	}
}

t_job* crearJob(t_job datos){
	t_job* nuevoJob= malloc(sizeof(t_job));
	setearJob(nuevoJob, datos);
	return nuevoJob;
}

void setearJob(t_job* nuevoJob, t_job datos){
	nuevoJob->bloque=datos.bloque;
	nuevoJob->master = datos.master;
	nuevoJob->nodo = datos.nodo;
	nuevoJob->cantidadTemporal = datos.cantidadTemporal;
	nuevoJob->estado=datos.estado;
	nuevoJob->etapa =datos.etapa;
	nuevoJob->temporal = malloc(strlen(datos.temporal) +1);
	strcpy(nuevoJob->temporal, datos.temporal);
	//asignar el resto de los camposa
}

void* buscar_por_nodo (int nodo, t_list* listaNodos){
	int es_el_nodo(t_job* job){
		return job->nodo == nodo;
	}
	return list_find(listaNodos, (void*) es_el_nodo);
}

void* buscar_por_jobid(int jobId){
	int _is_the_one(t_estados *p) {
		return p->job == jobId;
	}
	return list_find(tabla_estados, (void*) _is_the_one);
}



void planificarBloque(t_tabla_planificacion tabla, int numeroBloque, t_archivoxnodo* archivo){

	int* numBloqueParaLista = malloc(sizeof(int));
	*numBloqueParaLista=numeroBloque;
	bool existeBloqueEnWorker(void* elem){
		return numeroBloque == ((t_infobloque*)elem)->bloqueRelativo;
	}

	bool workerContieneBloque(void* elem){
		return string_equals_ignore_case(((t_clock*)tabla.clock_actual->data)->worker_id , ((t_nodoxbloques*)elem)->idNodo) &&
				list_any_satisfy(((t_nodoxbloques*)elem)->bloques, existeBloqueEnWorker);
	}

	void asignarBloqueAWorker(t_clock* worker,int* numeroBloque, t_archivoxnodo* archivo){
		worker->disponibilidad--;

		list_add(worker->bloques, numBloqueParaLista);

		bool existeWorkerAsignado(void* elem){
			return string_equals_ignore_case(((t_clock*)elem)->worker_id , worker->worker_id);
		}

		if(!list_any_satisfy(archivo->workersAsignados, existeWorkerAsignado))
		{
			list_add(archivo->workersAsignados, worker);
		}

	}

	if(list_any_satisfy(archivo->nodos, workerContieneBloque)){
		if(((t_clock*)tabla.clock_actual->data)->disponibilidad > 0)
		{
			asignarBloqueAWorker(((t_clock*)tabla.clock_actual->data), numBloqueParaLista, archivo);
			tabla.clock_actual = tabla.clock_actual->next;
			if(tabla.clock_actual == NULL)
				tabla.clock_actual = tabla.workers->head;

			if(((t_clock*)tabla.clock_actual->data)->disponibilidad ==0)
			{
				((t_clock*)tabla.clock_actual->data)->disponibilidad= configuracion.DISPONIBILIDAD_BASE;
				//Vuelve a mover el clock
				tabla.clock_actual = tabla.clock_actual->next;
				if(tabla.clock_actual == NULL)
					tabla.clock_actual = tabla.workers->head;
			}
		}
		else {
			//ACA NO DEBERIA ENTRAR NUNCA.
			printf("el worker apuntado por el clock actual posee el bloque pero no tiene disponibilidad.\n");
		}
	}
	else{ //el worker apuntado por el clock actual no posee ese bloque
		t_link_element* elementoActual= tabla.clock_actual->next;
		t_link_element* elementoOriginal = tabla.clock_actual;
		bool encontro=false;
		while(!encontro)
		{
			char* nombreWorker= ((t_clock*)elementoActual->data)->worker_id;
			bool workerContieneBloqueByWorkerId(void* elem){
				return string_equals_ignore_case( nombreWorker, ((t_nodoxbloques*)elem)->idNodo) &&
				list_any_satisfy(((t_nodoxbloques*)elem)->bloques, existeBloqueEnWorker);
			}
			if(list_any_satisfy(archivo->nodos, workerContieneBloqueByWorkerId) && ((t_clock*)elementoActual->data)->disponibilidad >0)
			{
				encontro=true;
				asignarBloqueAWorker(((t_clock*)elementoActual->data), numBloqueParaLista, archivo);
			}
			else{
				elementoActual = elementoActual->next;
				if(elementoActual == NULL)
					elementoActual = tabla.workers->head;

				if(string_equals_ignore_case(((t_clock*)elementoActual->data)->worker_id, ((t_clock*)elementoOriginal->data)->worker_id))
				{
					void sumarDisponibilidadBase(void* elem){
						((t_clock*)elem)->disponibilidad+= configuracion.DISPONIBILIDAD_BASE;
					}
					list_iterate(tabla.workers,sumarDisponibilidadBase);
				}
			}

		}

	}



}

