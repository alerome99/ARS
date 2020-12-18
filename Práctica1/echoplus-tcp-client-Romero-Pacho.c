// Practica tema 6, Romero Pacho Alejandro
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
	int s = socket(AF_INET, SOCK_STREAM, 0);	
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
		printf("Se ha pasado del numero limite de caracteres\n");
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

		//conecta el socket con el servidor
		int c = connect(s,(struct sockaddr*) &myaddr, sizeof(myaddr));
		//comprobacion de que la conexion se ha realizado de manera correcta
		if(c<0){
			printf("Fallo en el connect\n");
			perror("connect()");
			exit(EXIT_FAILURE);
		}
		
		//envio del cliente al servidor 
		int sendd = send(s,argv[argc-1], strlen(argv[argc-1]),0);
		//comprobacion de que el envio se ha realizado de manera correcta
		if (sendd<0){
			printf("Fallo en el envio\n");
			perror("send");
			exit(EXIT_FAILURE);
		}
				
		//recepcion de la cadena enviada por el servidor
		int recvv = recv(s,cadena, BUFFERSIZE, 0);
		//comprobacion de que la recepcion se ha realizado de manera correcta
		if (recvv<0){
			printf("Fallo en la recepcion\n");
			perror("recv");
			exit(EXIT_FAILURE);
		}
		printf("%s\n", cadena);
		//notifica al servidor de cerrar la conexion
		//al ser RDWR se desactiva tanto la recepcion como la emision
		int sh = shutdown(s, SHUT_RDWR);
		//comprobacion de que el shutdown se ha realizado de manera correcta
		if(sh<0){
			perror("shutdown()");
			close(s);
			exit(EXIT_FAILURE);
		}

		//como se recomienda en los apuntes tras un shutdown es recomendable hacer otro recv
		//este devolvera 0 bytes indicando que el otro extremo ha ceraddo la conexion tambien
		recvv = recv(s, cadena, BUFFERSIZE, 0);
		//comprueba que se hayan recivido 0 bytes
		if(recvv!=0){
			perror("recv");
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);
	//cierra el socket
	close(s);
	return 0;
}	 
