//ecoHilosServidor.c
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "./MensajeError.h"


void* SocketHandler(void*);
void manejadorTCPCliente(int);
void codigoMulticast();
void codigoTCP();

int host_port;

void codigoTCP(){

}

void codigoMulticast(){
	int host_port= 1101;

	struct sockaddr_in my_addr;

	int hsock;
	int * p_int ;
	int err;

	socklen_t addr_size = 0;
	int* csock;
	struct sockaddr_in sadr;
	pthread_t thread_id=0;


	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializando el socket %d\n", errno);
		goto FINISH;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
		
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error configurando opciones %d\n", errno);
		free(p_int);
		goto FINISH;
	}
	free(p_int);

	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = INADDR_ANY ;
	
	if( bind( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
		fprintf(stderr,"Error ligando el socket %d\n",errno);
		goto FINISH;
	}
	if(listen( hsock, 10) == -1 ){
		fprintf(stderr, "Error en el listen%d\n",errno);
		goto FINISH;
	}
	
	addr_size = sizeof(struct sockaddr_in);
	
	for(;;){
		printf("Esperando una conexión\n");
		csock = (int*)malloc(sizeof(int));
		if((*csock = accept( hsock, (struct sockaddr*)&sadr, &addr_size))!= -1){
			printf("Conexión recibida desde: %s\n",inet_ntoa(sadr.sin_addr));
			pthread_create(&thread_id,0,&SocketHandler, (void*)csock );
			pthread_detach(thread_id);
		}
		else{
			fprintf(stderr, "Error en el accept %d\n", errno);
		}
	}
	
FINISH:
;
}


void*codigo_del_hilo(void *id)
{
	codigoMulticast();
	//codigoTCP();

	pthread_exit(id);//terminar hilo
}

int main(int argc, char** argv){
	if(argc != 3) //Revisamos el número de argumentos
        mensajeFinalError("Uso: BucketSortServidor noServidores [<puerto>]");
	host_port= atoi(argv[2]);

	pthread_t hilos[atoi(argv[1])];//arreglo de datos tipo identificador de hilo
	int NUM_HILOS = atoi(argv[1]);
	int error;
	int *salida;
	int h;

	int id[NUM_HILOS];


	for(h=0;h<NUM_HILOS;h++)//crear hilos. Con este ciclo se crea la concurrencia de los hilos
	{
		id[h]=h;
		error=pthread_create(&hilos[h],NULL,codigo_del_hilo,&id[h]);
		if(error)
		{
			fprintf(stderr,"Error %d:%s\n",error,strerror(error)); 
			exit(-1);
		}
	}
	for(h=0;h<NUM_HILOS;h++)//unir los hilos. ESpero a que termine. genero un bloqueo hasta que uno de los hilos termine
	{
		error=pthread_join(hilos[h],(void**)&salida);
		if(error)
		{
			fprintf(stderr,"Error %d:%s\n",error,strerror(error));
		}
		else
		{
			printf("Hilos %d terminado\n",*salida);
		}
	}
}

void* SocketHandler(void* lp){
    int *csock = (int*)lp;

	char buffer[1024];
	int buffer_len = 1024;
	int bytecount;

	memset(buffer, 0, buffer_len);

	printf("Bytes recibidos %d\nCadena recibida: \"%s\"\n", bytecount, buffer);
	int dato = host_port;
	char cadena[25];
	sprintf(cadena,"%d",dato);
	strcat(buffer,cadena);
	host_port++;
	//Envio del puerto por multicast a cada hilo del cliente
	if((bytecount = send(*csock, buffer, strlen(buffer), 0))== -1){
		fprintf(stderr, "Error recibiendo datos %d\n", errno);
		goto FINISH;
	}
	
	printf("Bytes enviados: %d\n", bytecount);


FINISH:
	free(csock);
    return 0;
}
