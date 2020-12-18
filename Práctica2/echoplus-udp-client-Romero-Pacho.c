// Practica tema 5, Romero Pacho Alejandro
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <string.h>
//capacidad de la cadena 
#define BUFFERSIZE 100

int main (int argc, char *argv[]){
	char cadena[BUFFERSIZE];
	//estructura sockaddr del servidor en la que vamos a guardar los datos de la direccion del servidor adecuada para sockets de red AF_INET
	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;

	//convierte la direccion ip en un numero de 32 bits y lo guarda en el campo sin_addr de la estructura myaddr
	inet_aton(argv[1], &myaddr.sin_addr);

	//si hay 5 argumentos el numero de puerto es el introducido al ejecutar el programa
	if(argc==5){
		int numPuerto;
		sscanf(argv[3], "%d", &numPuerto);
		myaddr.sin_port = htons(numPuerto);
	}
	//si no hay 5 querra decir que no se ha introducido numero de puerto, por lo que se le dara el valor 5 de manera predeterminada	
	if(argc==3){
		int numPuerto=5;
		myaddr.sin_port = htons(numPuerto);
	}
	if(argc!=3 && argc!=5){
		printf("No se han introducido los parametros de manera correcta\n");
		return 0;
	}

	//abre el socket con ayuda de la funcion socket
	int s = socket(AF_INET, SOCK_DGRAM, 0);	
	//comprobacion de que el socket se ha creado de manera correcta
	if (s<0){
		printf("Fallo en la creacion del socket\n");
		perror("socket()");
		exit(EXIT_FAILURE);
	}
	//al igual que con la estructura del servidor se crea una para el cliente y se rellenan sus campos	
	struct sockaddr_in myaddrclient;
	myaddrclient.sin_family=AF_INET;
	myaddrclient.sin_port=0;
	myaddrclient.sin_addr.s_addr=INADDR_ANY;

	//si la cadena que hay que pasar a mayusculas tiene mas de 80 caracteres saltara error y no se realizara el envio
	if(strlen(argv[argc-1])>80){
		printf("Se ha pasado del numero limite de caracteres");
	}
	//en caso de ser de 80 o menos caracteres continuara
	else{
		//enlaza el socket con la direccion
		int b = bind(s, (struct sockaddr*) &myaddrclient, sizeof(myaddrclient));
		//comprueba que el bind se ha realizado de manera correcta
		if (b<0){
			printf("Fallo en el bind\n");
			perror("bind()");
			exit (EXIT_FAILURE);
		}
		//envio del cliente al servidor 
		int send = sendto(s,argv[argc-1], strlen(argv[argc-1]),0, (struct sockaddr*)&myaddr, sizeof(myaddr));
		//comprobacion de que el envio se ha realizado de manera correcta
		if (send<0){
			printf("Fallo en el envio\n");
			perror("sendto()");
			exit(EXIT_FAILURE);
		}
		
		//tamaño de la estructura sockaddr del servidor
		socklen_t len = sizeof(myaddr); 
		
		//recepcion de la cadena enviada por el servidor
		int recv = recvfrom(s,cadena, BUFFERSIZE, 0,(struct sockaddr*)&myaddrclient,&len);
		//comprobacion de que la recepcion se ha realizado de manera correcta
		if (recv<0){
			printf("Fallo en la recepcion\n");
			perror("recvfrom()");
			exit(EXIT_FAILURE);
		}
	}
	printf("%s\n",cadena);
	return 0;
}	 