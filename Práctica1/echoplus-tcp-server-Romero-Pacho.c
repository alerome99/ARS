// Practica tema 6, Romero Pacho Alejandro
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

//variable del socket
int s;
//para poder interrumpir el programa
//cerramos el socket del padre
void signal_handler(int signal){
	if(signal==SIGINT){
		int shPadre = shutdown(s,2);
		if(shPadre<0){
			perror("shutdown()");
			exit(EXIT_FAILURE);
		}
		int clPadre = close(s);
		if(clPadre<0){
			perror("close()");
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}
}

int main (int argc, char* argv[]){
	//llamada a signal
	signal(SIGINT, signal_handler);
	//estructura sockaddr del cliente
	struct sockaddr_in myaddrClient;
	//estructura sockaddr del servidor
	struct sockaddr_in myaddrServer;
	char cadena[BUFFERSIZE];
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
	s = socket(AF_INET, SOCK_STREAM,0);
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
	//marca un socket como de apertura pasiva, preparado para recibir conexiones
	//el segundo argumento es el tamaño maximo de la cola de conexiones pendientes de aceptar
	int l = listen(s, 30);
	//comprobacion de que el listen se esta realizando de manera correcta
	if(l<0){
		perror("listen()");
		exit(EXIT_FAILURE);
	}
	//tamaño de la estructura sockaddr del cleinte
	socklen_t len = sizeof(myaddrClient);
	//variable para comprobacion de errores de send, recv, accept ,fork, shutdown y close del accept
	//y variable indice para iterar 
	int a, sendd, recvv, pid, sh, cl, indice;
	//el programa se mantendra escuchando solicitudes hasta que se corte su ejecucion
	while(1){
		//para evitar que queden cadenas en memoria, cuando enviaba una cadena grande y luego una mas pequeña el servidor seguia enviando de vuelta la grande
		//con esto limpiara el vector por cada solicitud procesada
		memset(cadena, 0, BUFFERSIZE);

		//espera la conexion de un cliente
		a = accept(s,(struct sockaddr*) &myaddrServer, &len);
		//comprueba que el accept se ha realizado de manera correcta
		if(a<0){
			perror("accept()");
			exit(EXIT_FAILURE);
		}

		//creacion de un proceso para procesar la solicitud entrante
		pid = fork();
		//comprobacion de que la creacion del proceso se ha realizado de manera correcta
		if(pid<0){
			perror("fork()");
			exit(EXIT_FAILURE);
		}
		//recepcion de la cadena enviada por el cliente
		recvv = recv(a,cadena,BUFFERSIZE,0);
		//comprobacion de que la recepcion se ha realizado de manera correcta
		if(recvv<0){
			perror("recv()");
			close(a);
			exit(EXIT_FAILURE);
		}

		//paso de minusculas a mayusculas
		for(indice=0; cadena[indice] != '\0'; ++indice){
			cadena[indice] = toupper(cadena[indice]);
		}

		//envio del servidor al cliente
		sendd = send(a,cadena,BUFFERSIZE,0);
		//comprobacion de que el envio se ha realizado de manera correcta
		if(sendd<0){
			perror("send()");
			exit(EXIT_FAILURE);
		}

		printf("Cadena enviada: %s\n", cadena);

		//notifica al servidor de cerrar la conexion
		//al ser RDWR se desactiva tanto la recepcion como la emision
		sh = shutdown(a, SHUT_RDWR);
		//comprobacion de que el shutdown se ha realizado de manera correcta
		if(sh<0){
			perror("shutdown()");
			close(a);
			exit(EXIT_FAILURE);
		}
			
		//como se recomienda en los apuntes tras un shutdown es recomendable hacer otro recv
		//este devolvera 0 bytes indicando que el otro extremo ha ceraddo la conexion tambien
		recvv = recv(a,cadena,BUFFERSIZE,0);
		//comprueba que se hayan recivido 0 bytes	
		if(recvv!=0){
			perror("recv()");
			close(a);
			exit(EXIT_FAILURE);
		}

		//cierra el accept
		cl = close(a);
		//comprueba que se ha realizado de manera correcta
		if(cl<0){
			perror("close()");
			exit(EXIT_FAILURE);
		}
	}		
}	
