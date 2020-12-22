// Practica tema 8, Romero Pacho Alejandro

#include "ip-icmp-ping.h" //fichero de cabecera usado para la ECHORequest y ECHOResponse
//IPs utilizadas para probar : 8.8.8.8 8.8.4.4 1.1.1.1 127.0.0.1

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[]){
	//variable para ver si se imprime el desarrollo del informe
	int v = 0;

	//comprobacion de que se han introducido de manera correcta los parametros en la ejecucion
	if(argc!=2 && argc!=3){
		printf("No se han introducido los parametros de manera correcta.\n");
		exit(EXIT_FAILURE);
	}

	//si se ha introducio -v quiere decir que se debe informar de cada uno de los pasos necesarios para enviar o recibir
	if(argc==3 && strcmp(argv[2],"-v")==0){
		v=1;
	}

	//estructura adecuada para sockets de red AF_INET y se rellenan sus campos
	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = 0;
	myaddr.sin_addr.s_addr = INADDR_ANY;

	//creacion del socket, como el cliente sera udp dgram en vez de stream
	//para icmp se usa SOCK_RAW
	int s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	//comprobacion de que se ha realizado de manera correcta la creacion del socket
	if(s<0){
		perror("socket()");
		exit(EXIT_FAILURE);
	}

	//enlaza el socket con la direccion
	int b = bind(s, (struct sockaddr *) &myaddr, sizeof(myaddr));
	//comprobacion que el bind se ha realizado de manera correcta
	if(b<0){
		perror("bind()");
		exit(EXIT_FAILURE);
	}

	//creacion de la solicitud echo
	//es decir la solicitud ICMP de ping
	ECHORequest solicitudICMP;

	if(v==1){
		printf("-> Generando cabecera ICMP.\n");
	}

	//añadir tipo a la solicitud
	//cabecera icmp
	solicitudICMP.icmpHeader.Type = 8;

	//impimir tipo de la solicitud
	if(v==1){
		printf("-> Type: %u\n",solicitudICMP.icmpHeader.Type);
	}

	//añadir codigo a la solicitud
	//cabecera icmp
	solicitudICMP.icmpHeader.Code = 0;

	//impimir codigo de la solicitud
	if(v==1){
		printf("-> Code: %u\n",solicitudICMP.icmpHeader.Code);
	}

	//inicializacion checksum
	//cabecera icmp
	solicitudICMP.icmpHeader.Checksum = 0;
	//añadir id de la solicitud
	solicitudICMP.ID = getpid();

	//imprimir id de la solicitud
	if(v==1){
		printf("-> Identifier (pid): %u.\n",solicitudICMP.ID);
	}

	//añadir numero de secuencia a la solicitud
	solicitudICMP.SeqNumber = 0;

	//imprimir numero de secuencia de la solicitud
	if(v==1){
		printf("-> Seq. number: %u\n",solicitudICMP.SeqNumber);
	}

	//reserve espacio en memoria y añadir valor al campo payload
	memset(solicitudICMP.payload, 0, 64);
	strcpy(solicitudICMP.payload, "Este es el payload");

	//imprimir el valor de payload
	if(v==1){
		printf("-> Cadena a enviar: %s.\n",solicitudICMP.payload);
	}

	//calculo del checksum
	//se inicializa a 0 los dos bytes del campo de checksum en el datagrama
	solicitudICMP.icmpHeader.Checksum = 0x00000000;
	//define una variable entera numShorts, i nicializandola al tamaño en bytes del datagrama, dividido entre dos
	//esto da el tamaño half word
	int numShorts = sizeof(solicitudICMP)/2;
	//se define un puntero a unsigned short int (para recorrer los elementos de 16 bits del datagrama ICMP)
	//se declara como puntero para que apunte a elementos de 16 bits consiguiendo asi que un incremento apunte alsiguiente elemento
	unsigned short int *puntero;
	//se define un acumulador de tipo unsigned int = 0, (variable de 32 bits sin signo para ir acumulando los resultados parciales)
	unsigned int acumulador = 0;
	//se inicializa el puntero al inicio del datagrama ICMP
	puntero = (unsigned short int*)&solicitudICMP;

	int i;
	//se calcular el valor del acumulador y el puntero
	for(i=0;i<numShorts-1;i++){
		acumulador=acumulador+(unsigned int)*puntero;
		//como puntero apunta a elementos de 16 bits, este incremento lo deja apuntando al siguiente elemento
		puntero++;
	}

	//sumar la parte alta del acumulador a la parte baja
	acumulador = (acumulador >> 16) + (acumulador & 0x0000ffff);
	//se suma dos veces por si se produce un nuevo acarreo de la acumulacion del acarreo a partir del 3º byte
    acumulador = (acumulador >> 16) + (acumulador & 0x0000ffff);
    //una vez hecho, simplemente hay que hacer un not del resultado, para obtener su complemento a 1, y guardar los 16 bits de menos peso en el campo de checksum
    acumulador = ~acumulador;
    //se añade el valor al campo checksum
    //cabecera icmp
	solicitudICMP.icmpHeader.Checksum = (short int)acumulador;

	//imprimir checksum
	if(v==1){
		printf("-> Checksum: 0x%x.\n",solicitudICMP.icmpHeader.Checksum);
	}

	//imprimir el tamaño de la solicitud
	if(v==1){
		printf("-> Tamanio total del paquete ICMP: %lu.\n",sizeof(solicitudICMP));
	}

	//convierte la direccion ip en un numero de 32 bits y lo guarda en el campo sin_addr de la estructura myaddr
	//por ultimo antes del envio rellenar la ip de la estructura para que el envio se haga de manera correcta
	int ia = inet_aton(argv[1], &myaddr.sin_addr);
	//comprobacion de que se ha realizado de manera correcta la asignacion de ip
	if(ia==0){
		perror("inet_aton()");
		exit(EXIT_FAILURE);
	}

	//envio de la solicitud echo
	int sendd = sendto(s, &solicitudICMP, sizeof(solicitudICMP), 0,(struct sockaddr *)&myaddr, sizeof(myaddr));
	//comprobacion de que se ha realizado de manera correcta el envio de la solicitud
	if(sendd<0){
		perror("sendto()");
		exit(EXIT_FAILURE);
	}

	printf("Paquete ICMP enviado a %s\n", argv[1]);

	//tamaño de la estructura sockaddr
	socklen_t len = sizeof(myaddr);

	//creacion de la respuesta echo
	//es decir la respuesta ICMP de ping
	ECHOResponse respuestaICMP;

	//recepcion de la respuesta echo
	int recvv = recvfrom(s, &respuestaICMP, sizeof(respuestaICMP), 0, (struct sockaddr *) &myaddr, &len);
	//comprobacion de que se ha realizado de manera correcta la recepcion de la respuesta
	if(recvv<0){
		perror("recvfrom()");
		exit(EXIT_FAILURE);
	}

	printf("Respuesta recibida desde %s\n", argv[1]);

	//los valores de los tipos segun la pagina consultada de wikipedia sobre icmp estan entre 0 y 255
	if(respuestaICMP.icmpHeader.Type > 255 || respuestaICMP.icmpHeader.Type < 0){
		printf("Tipo no contemplado.\n");
		exit(EXIT_FAILURE);	
	}

	//los valores de los codigos segun la pagina consultada de wikipedia sobre icmp estan entre 0 y 15
	if(respuestaICMP.icmpHeader.Code > 15 || respuestaICMP.icmpHeader.Code < 0){
		printf("Codigo no contemplado.\n");
		exit(EXIT_FAILURE);	
	}

	//con esto podremos saber segun el tipo y el codigo de la respuesta el estado de la red
	//impresion de la respuesta
	//segun el valor del campo tipo sera una respuesta
	//y el codigo nos dice la respuesta que es dentro de ese tipo
	switch(respuestaICMP.icmpHeader.Type) {
		//echo reply
        case 0 :
            if (v==1){
                printf("-> Tamanio de la respuesta: %d\n", recvv);
                printf("-> Cadena recibida: %s.\n", respuestaICMP.payload);
                printf("-> Identifier (pid): %u.\n", respuestaICMP.ID);
                printf("-> TTL: %u.\n", respuestaICMP.ipHeader.TTL);
            }
            printf("Descripcion de la respuesta: respuesta correcta (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
            break;
        case 3:
            printf("Destination unreachable: ");
            switch(respuestaICMP.icmpHeader.Code){
                case 0:
                    printf("Destination network unreachable (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 1:
                    printf("Destination host unreachable (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 2:
                    printf("Destination protocol unreachable (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 3:
                    printf("Destination port unreachable (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 4:
                    printf("Fragmentation required, and DF flag set (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 5:
                    printf("Source route failed (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 6:
                    printf("Destination network unknown (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 7:
                    printf("Destination host unknown (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 8:
                    printf("Source host isolated (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 9:
                    printf("Network administratively prohibited (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 10:
                    printf("Host administratively prohibited (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 11:
                    printf("Network unreachable for ToS. (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 12:
                    printf("Host unreachable for ToS (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 13:
                    printf("Communication administratively prohibited (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 14:
                    printf("Host precedence violation (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
                case 15:
                    printf("Precedence cutoff in effect (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
                    break;
        	}
        	break;
        //4 deprecated
        case 5:
	    printf("Redirect Message: ");
	    	switch(respuestaICMP.icmpHeader.Code){
		    	case 0:
		    		printf("Redirect Datagram for the Network (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
		    		break;
			    case 1:
				    printf("Redirect Datagram for the Host (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
				    break;
			    case 2:
				    printf("Redirect Datagram for the ToS & network (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
				    break;
			    case 3:
				    printf("Redirect Datagram for the ToS & host (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
				    break;
	    	}
	    	break;
	   	//6 deprecated y 7 unassigned
	    case 8:
	    	printf("Echo request (used to ping) (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
	    	break;
	    case 9:
		    printf("Router Advertisement (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
		    break;
		case 10:
		    printf("Router Solicitation: Router discovery/selection/solicitation (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
		    break;
				
        case 11:
            printf("Time exceeded: ");
            switch(respuestaICMP.icmpHeader.Code){
                case 0:
                    printf("TTL expired in transit (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
                    break;
                case 1:
                    printf("Fragment reassembly (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
                    break;
            }
            break;

        case 12:
            printf("Parameter problem. Bad IP header: ");
            switch(respuestaICMP.icmpHeader.Code){
                case 0:
                    printf("Pointer indicates the error (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
                    break;
                case 1:
                    printf("Missing a required option (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
                    break;
                case 2:
                    printf("Bad length (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
                    break;
            }
            break;

		case 13:
		    printf("Timestamp (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
		    break;
		case 14:
		    printf("Timestamp Reply (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type );
		    break;

		//15 a 18 deprecated
		//Reservados
		case 19:
			printf("Reserved for security (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 20:
			printf("Reserved for robustness experiment (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 21:
			printf("Reserved for robustness experiment (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 22:
			printf("Reserved for robustness experiment (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 23:
			printf("Reserved for robustness experiment (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 24:
			printf("Reserved for robustness experiment (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 25:
			printf("Reserved for robustness experiment (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 26:
			printf("Reserved for robustness experiment (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 27:
			printf("Reserved for robustness experiment (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 28:
			printf("Reserved for robustness experiment (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 29:
			printf("Reserved for robustness experiment (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;

		//30 a 39 deprecated
		case 40:
			printf("Photuris, Security failures (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;

		//Experimental
		case 41:
			printf("ICMP for experimental mobility protocols such as Seamoby [RFC4065] (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;

		//Extended request and reply
		case 42:
			printf("Extended Echo Request: Request Extended Echo (XPing - see Extended Ping (Xping)) (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 43:
			printf("Extended Echo Reply: ");
			switch(respuestaICMP.icmpHeader.Code){
				case 0:
					printf("No Error (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
					break;
				case 1:
					printf("Malformed Query (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
					break;
				case 2:
					printf("No Such Interface (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
					break;
				case 3:
					printf("No Such Table Entry (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
					break;
				case 4:	
					printf("Multiple Interfaces Satisfy Query (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			}
			break;
		//44 a 252 unassigned	
		case 253:
			printf("RFC3692-style Experiment 1 (RFC 4727) (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 254:
			printf("RFC3692-style Experiment 2 (RFC 4727) (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
		case 255:
			printf("Reserved (type %d, code %d). \n",respuestaICMP.icmpHeader.Code,respuestaICMP.icmpHeader.Type);
			break;
    }
	return 0;
}