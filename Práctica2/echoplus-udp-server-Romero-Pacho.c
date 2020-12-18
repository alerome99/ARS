// Practica tema 5, Romero Pacho Alejandro
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
//capacidad de la cadena 
#define BUFFERSIZE 81

int main (int argc, char* argv[]){
	//estructura sockaddr del cliente
	struct sockaddr_in myaddrClient;
	//estructura sockaddr del servidor
	struct sockaddr_in myaddrServer;
	char cadena[BUFFERSIZE];
	//variable para comprobacion de errores de sendto y recvfrom
	int sent;
	int recv;
	//si se han introducido 3 parametros se tomara como numero de puerto el tercer argumento
	if (argc==3){
		int numPuerto;
		sscanf(argv[2], "%d", &numPuerto);
		myaddrServer.sin_port = htons(numPuerto);	
	//en caso de introducirse menos de 3 se pondra como numero de puerto el 5
	}else{
		int numPuerto=5;
		myaddrServer.sin_port = htons(numPuerto);
	}

	//abre el socket con ayuda de la funcion socket
	int s = socket(AF_INET, SOCK_DGRAM,0);
	//comprobacion de que el socket se ha creado de manera correcta
	if(s<0){
		perror("socket()");
		exit(EXIT_FAILURE);
	}
	//rellena los campos del sockaddr del servidor
	myaddrServer.sin_family = AF_INET;
	//INADDR_ANY representa cualquier IP de todos los interfaces red de tu ordenador
	myaddrServer.sin_addr.s_addr = INADDR_ANY;

	//enlaza el socket con la direccion
	int b = bind(s,(struct sockaddr*) &myaddrServer,sizeof(myaddrServer));
	//comprueba que el bind se ha realizado de manera correcta
	if (b<0){
		perror("bind()");
		exit(EXIT_FAILURE);
	}
	char hostname [HOST_NAME_MAX];
	int h = gethostname(hostname, HOST_NAME_MAX);
	if (h<0){
		perror("gethostname()");
		exit(EXIT_FAILURE);
	}
	//tamaño de la estructura sockaddr del cleinte
	socklen_t len = sizeof(myaddrClient);
	
	//El programa se mantendra escuchando solicitudes hasta que se corte su ejecucion
	while(1){
		//Para evitar que queden cadenas en memoria, cuando enviaba una cadena grande y luego una mas pequeña el servidor seguia enviando de vuelta la grande
		//Con esto limpiara el vector por cada solicitud procesada
		memset(cadena, 0, BUFFERSIZE);
		//recepcion de la cadena enviada por el cliente
		recv = recvfrom(s, cadena, BUFFERSIZE,MSG_WAITALL, (struct sockaddr*) &myaddrClient, (socklen_t *) &len);
		//comprobacion de que la recepcion se ha realizado de manera correcta
		if(recv < 0){
			perror("recvfrom()");
			exit(EXIT_FAILURE);
		}
		//paso de minusculas a mayusculas
		int indice;	
		for(indice=0; cadena[indice] != '\0'; ++indice){
			cadena[indice] = toupper(cadena[indice]);
		}
		//envio del servidor al cliente 
		sent = sendto(s, cadena,BUFFERSIZE,0,(struct sockaddr*)&myaddrClient,sizeof(myaddrClient));
		//comprobacion de que el envio se ha realizado de manera correcta
		if(sent < 0){
			perror("sendto()");
			exit(EXIT_FAILURE);
		}

		printf("Cadena enviada: %s\n", cadena);
	}
}	