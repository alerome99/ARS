// Practica tema 7, Romero Pacho Alejandro

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int main (int argc, char *argv[]){
	//estructura sockaddr del servidor en la que vamos a guardar los datos de la direccion del servidor 
	struct sockaddr_in myaddr;
	int v = 0;
	//tamaño del buffer
	char data[516];
	char ack[4];
	int numeroBloque;
	int s;
	int numeroBloqueEsperado = 1;

	//comprobacion de que se han introducido de manera correcta los parametros
	if(argc!=4 && argc!=5){
		printf("No se han introducido los parametros de manera correcta\n");
		return 0;
	}else{
		//estructura adecuada para sockets de red AF_INET
		myaddr.sin_family = AF_INET;
		//convierte la direccion ip en un numero de 32 bits y lo guarda en el campo sin_addr de la estructura myaddr
		inet_aton(argv[1], &myaddr.sin_addr);
		struct servent* se;
		//devuelve estructura servent de la base de datos que coincide con el nombre del servicio (tftp) mediante el protocolo (udp)
		se = getservbyname("tftp","udp");
		//comprobacion de que se ha realizado de manera correcta el get 
		//si es nulo quiere decir que no lo ha encontrado
		if(se == NULL){
			perror("getservbyname()");
			exit(EXIT_FAILURE);
		}

		//si se ha introducio -v quiere decir que se debe informar de cada uno de los pasos necesarios para enviar o recibir
		if(argc==5 && strcmp(argv[4], "-v")==0){
			v=1;
		}

		myaddr.sin_port = se->s_port;
		//creacion del socket, como el cliente sera udp dgram en vez de stream
		s = socket(AF_INET, SOCK_DGRAM, 0);
		//comprobacio de que se ha realizado de manera correcta la creacion del socket
		if(s<0){
			perror("socket()");
			exit(EXIT_FAILURE);
		}
	}

	//comprobar que el nombre del archivo tenga menos de 100 caracteres
	if(strlen(argv[3]) > 100){
		printf("Se ha pasado del numero limite de caracteres\n");
		return 0;
	}else{
		//declaracion del tamaño del array de solicitud de lectura y escritura
		char solicitud[strlen("octet") + strlen(argv[3]) + 4];
		//si es de lectura el op code es 01
		if(strcmp(argv[2],"-r")==0){
			solicitud[0] = 0;
			solicitud[1] = 1;
		}
		//si es de escritura el op code es 02
		else{
			solicitud[0] = 0;
			solicitud[1] = 2;
		}
		//añadir nombre del archivo
		strcpy(&solicitud[2],argv[3]);
		//añadir modo de transmision
	   	strcpy(&solicitud[(strlen(argv[3]))+3],"octet");

		//en caso de que se introduzca -r
		//lectura
		if(strcmp(argv[2],"-r")==0){
			//envio udp de la solicitud de lectura
			int sendd = sendto(s, solicitud, 128, 0, (struct sockaddr*)&myaddr, sizeof(myaddr));
			//comprobacion de que se ha realizado de manera correcta el envio
			if(sendd<0){
				perror("sendto() PRQ");
				exit(EXIT_FAILURE);
			}
			//se abre el fichero en modo escritura, ya que se va a escribir sobre el lo que recibamos
			//al no existir lo va a crear
			FILE *fichero = fopen(argv[3], "w");

			int recvv;
			socklen_t len = sizeof(myaddr); 

			//si se ha añadido -v al ejecutar el programa, indica que se ha enviado la solicitud del archivo de nombre 'x' a la ip 'y' 
			if(v==1){
				printf("Enviada solicitud de lectura de %s a servidor TFTP en %s.\n", argv[3], argv[1]);
			}
			//rellenamos el op code del ACK
			ack[0]=0;
			ack[1]=4;

			//se seguira haciendo hasta que acabe de leer los datos del archivo del servidor
			while(1){
				//recepcion udp
				recvv=recvfrom(s, data, 516, 0, (struct sockaddr*)&myaddr,&len);
				//comprobacion de que se ha realizado de manera correcta el recibo
				if(recvv<0){
					perror("recvfrom() data");
					exit(EXIT_FAILURE);
				}
				//calcular numero de bloque
				numeroBloque = (unsigned char)data[2]*256+(unsigned char)data[3];
				//comprobacion de que la recepcion de bloques se esta haciendo en orden
				if(numeroBloque!=numeroBloqueEsperado){
					printf("Se han recibido los bloques en un orden equivocado.\n");
					exit(EXIT_FAILURE);
				}
				numeroBloqueEsperado++;
				//si hay un problema el servidor enviara un codigo de error en un paquete con opcode 5
				if(data[1]==5){
					//codigos de error devueltos por el servidor TFTP
					switch(data[3]){
						case 0:
						printf("No definido. Comprobar errstring\n");
						break;
					case 1:
						printf("Fichero no encontrado\n");
						break;
					case 2:
						printf("Violacion de acceso\n");
						break;
					case 3:
						printf("Espacio de almacenamiento lleno\n");
						break;
					case 4:
						printf("Operacion TFTP ilegal\n");
						break;
					case 5:
						printf("Identificador de transferencia desconocido\n");
						break;
					case 6:
						printf("El fichero ya existe \n");
						break;
					case 7:
						printf("Usuario desconocido\n");
						break;
					}
				exit(EXIT_FAILURE);
				}
				//si se ha añadido -v al ejecutar el programa, indica que se ha recibido el bloque del servidor
				if(v==1){
					printf("Recibido bloque del servidor TFTP.\n");
				}
				//si se ha añadido -v al ejecutar el programa, indica que bloque era el que se ha recibido del servidor
				if(v==1){
					if(numeroBloque==1){
						printf("Es el primer bloque (numero de bloque 1).\n");
					}else{		
						printf("Es el bloque con codigo %i.\n", numeroBloque);
					}					
				}

				//guardar los datos del fichero del servidor en el fichero local
				fwrite(data+4, 1, recvv-4,fichero);

				//rellenamos el numero de bloque del ACK
				ack[2]=data[2];
				ack[3]=data[3];
				//envio del ack
				sendd = sendto(s, ack, 4, 0, (struct sockaddr*)&myaddr, sizeof(myaddr));
				//comprobacion de que el envio del ack se ha realizado de manera correcta
				if(sendd<0){
					perror("sendto() ack");
					exit(EXIT_FAILURE);
				}
				//si se ha añadido -v al ejecutar el programa, indica de que bloque es el ack enviado
				if(v==1){
					printf("Enviamos el ACK del bloque %i.\n",numeroBloque);
				}		
				//el protocolo define que el último bloque de datos tendrá un tamaño estrictamente menor de 512 bytes
				//al no ser completo el ultimo bloque, sabremos que su tamaño sera menos que 516 bytes, luego si el recibo es != 516 sabremos que es el ultimo bloque
				if(recvv!=516){
					//si se ha añadido -v al ejecutar el programa, indica el cierre del fichero y cual fue el ultimo bloque
					if(v==1){
						printf("El bloque %i era el ultimo: cerramos el fichero.\n",numeroBloque);
					}
					//cierre del fichero
					fclose(fichero);
					exit(EXIT_SUCCESS);
				}		
			}					
		}

		//en caso de que se introduzca -w
		//modo escritura
		if(strcmp(argv[2],"-w")==0){
			//envio udp de la solicitud de escritura
			int sendd = sendto(s, solicitud, 128, 0, (struct sockaddr*)&myaddr, sizeof(myaddr));
			//comprobacion de que se ha realizado bien el envio
			if(sendd<0){
				perror("sendto() WRQ");
				exit(EXIT_FAILURE);
			}

			//se abre el fichero en modo lectura, ya que se va a leer lo que esta escrito
			FILE *fichero = fopen(argv[3],"r");
			//comprobacion de que se ha abierto de manera correcta
			if(fichero==NULL){
				printf("No existe un fichero con ese nombre.\n");
				exit(EXIT_FAILURE);
			}
			//si se ha añadido -v al ejecutar el programa, indica que se ha enviado la solicitud de escritura del archivo 'x' a la ip 'y'
			if(v==1){
				printf("Enviada solicitud de escritura de %s a servidor tftp en %s. \n", argv[3], argv[1]);
			}

			int i = 0;
			//rellenamos el op code del buffer de datos
			data[0]=0;
			data[1]=3;

			//para recorrer el fichero
			while(!feof(fichero)){
				//lectura del fichero
				//rellenamos el buffer con los caracteres del fichero
				//cada bloque se sobreescribira el buffer
				data[i+4]=fgetc(fichero);
				i++;
				//se hara un envio cada vez que se llene un bloque o cuando llegue al final del fichero
				if(i%512==0 || feof(fichero)){
					socklen_t len = sizeof(myaddr);
					//recepcion del ack
					int recvv = recvfrom(s, ack, 4, 0, (struct sockaddr*)&myaddr,&len);
					//comprobacion de que la recepcion del ack se ha hecho de manera correcta
					if(recvv<0){
						perror("recvfrom() ack");
						exit(EXIT_FAILURE);
					}

					//calcular numero de bloque
					numeroBloque = (unsigned char)ack[2]*256+(unsigned char)ack[3]+1;
					//comprobacion de que la recepcion de bloques se esta haciendo en orden
					if(numeroBloque!=numeroBloqueEsperado){
						printf("Se han recibido los bloques en un orden equivocado.\n");
						exit(EXIT_FAILURE);
					}
					numeroBloqueEsperado++;
					//si hay un problema el servidor enviara un codigo de error en un paquete con opcode 5
					if(ack[1]==5){
					//codigos de error devueltos por el servidor TFTP
						//el codigo de error se almacena en el 2 y 3 byte, puesto que codigo de error < 256 nos valdra con el byte 3
						switch(ack[3]){
							case 0:
								printf("No definido. Comprobar errstring\n");
								break;
							case 1:
								printf("Fichero no encontrado\n");
								break;
							case 2:
								printf("Violacion de acceso\n");
								break;
							case 3:
								printf("Espacio de almacenamiento lleno\n");
								break;
							case 4:
								printf("Operacion TFTP ilegal\n");
								break;
							case 5:
								printf("Identificador de transferencia desconocido\n");
								break;
							case 6:
								printf("El fichero ya existe \n");
								break;
							case 7:
								printf("Usuario desconocido\n");
								break;
						}
						exit(EXIT_FAILURE);
					}
					//si se ha añadido -v al ejecutar el programa, indica que se ha recibido el ack de cada bloque
					if(v==1){
						printf("ACK recibido del bloque %i. \n",numeroBloque);
					}

					//numero de bloque
					//ya que a veces el numero de bloque sera muy grande como para entrar en 2 bytes sera necesario desplazarlo
					//desplazar 8 bits a la derecha es lo mismo que dividir entre 256 
					//asi el numero de bloque estara representado en 16 bits los 8 de menor valor son data[3] y los 8 de mayor son data[1]
					//si data[2] = 2 y data[3] = 1 el numero de bloque sera = 0000 0010 0000 0001 que es 513
					data[2]=(numeroBloque >> 8); 
					data[3]=numeroBloque%256;

					//calculo del tamaño del envio
					//si el bloque se ha enviado entero el tamaño de envio sera 516, el bloque al completo, los 512 + 4, es decir i+4
					//en cambio si se llega al envio por fin de fichero y no al completarse el bloque habra que calcularlo
					//este sera el resultado de sumar el resto de dividir el valor de i entre 512 que es el tamaño maximo de los datos y 3
					//puesto que i%512=i el tamaño de envio sera siempre i+3 en este caso, ya que al no llegar al final del array de datos no se sumara el fin de fichero
					if(i%512!=0){
						i--;
					}
					//envio de los datos
					sendd = sendto(s, data, i+4, 0, (struct sockaddr*)&myaddr,sizeof(myaddr));
					//comprobacio de que se han enviado bien los datos
					if(sendd<0){
						perror("sendto() data");
						exit(EXIT_FAILURE);
					}

					//resetear el valor de i para que siempre este entre 0 y 512, si no lo reseteamos a 0 la manera de calcular el tamaño de envio seria igulandolo a 512 en caso
					//de que se envie el bloque completo, y en caso de que no se enviara completo habria que enviar i%512
					i=0;
				}
			}
			//si se ha añadido -v al ejecutar el programa, indica que bloque fue el ultimo
			if(v==1){
				printf("El bloque %i era el ultimo: cerramos el fichero.\n",numeroBloque);
			}
			//cierre del fichero
			fclose(fichero);
		}
	}
	return 0;
}