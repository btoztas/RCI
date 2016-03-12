#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define PORT 58000
#define STDIN 0

#define NAME 0
#define IP 1


int newudpclient(struct sockaddr_in *serveraddr, char *name, int chartype, char *port){

  int fd;
  struct hostent *h;

  fd=socket(AF_INET, SOCK_DGRAM, 0);
  if(fd==-1){
    printf("Error opening socket\n");
    exit(1);
  }

  memset((void*)&(*serveraddr),(int)'\0',sizeof((*serveraddr)));
  (*serveraddr).sin_family=AF_INET;

  if(chartype == NAME){
	  if((h=gethostbyname(name))==NULL){
	    printf("Error on getting host\n");
	    exit(1);
	  }
  	(*serveraddr).sin_addr.s_addr=((struct in_addr*)(h->h_addr_list[0]))->s_addr;
	}else{
		inet_pton(AF_INET, name, &((*serveraddr).sin_addr));
		printf("oi\n");
	}
  (*serveraddr).sin_port=htons((u_short)atoi(port)); 
  printf("2oi\n");
  return fd;
}

void QRY(char parametros[128], char *ip, char *port){
  
  int n;
  int addrlen;
  char buffer[128];

  addrlen = sizeof((serveraddr));
  sprintf(buffer, "REG %s.%s;%s;%s", name, surname, ip, scport);
  printf("Mensagem enviada para o servidor: %s\n", buffer);
  if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&(serveraddr), addrlen)==-1){
    printf("Error sending\n");
    exit(1);
  }
  if((n=recvfrom(socketfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&(serveraddr), &addrlen))==-1){
    printf("Error on recvfrom\n");
    exit(1);
  }
  buffer[n]='\0';
  printf("%s\n",buffer);
}

void REG(char name[128], char surname[128], char ip[128], char scport[128], int socketfd, struct sockaddr_in serveraddr){
  
  int n;
  int addrlen;
  char buffer[128];

  addrlen = sizeof((serveraddr));
  sprintf(buffer, "REG %s.%s;%s;%s", name, surname, ip, scport);
  printf("Mensagem enviada para o servidor: %s\n", buffer);
  if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&(serveraddr), addrlen)==-1){
    printf("Error sending\n");
    exit(1);
  }
  if((n=recvfrom(socketfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&(serveraddr), &addrlen))==-1){
    printf("Error on recvfrom\n");
    exit(1);
  }
  buffer[n]='\0';
  printf("%s\n",buffer);
}

void UNR(char name[128], char surname[128], int socketfd, struct sockaddr_in serveraddr){
  
  int n;
  int addrlen;
  char buffer[128];

  addrlen = sizeof((serveraddr));
  sprintf(buffer, "UNR %s.%s", name, surname);
  printf("Mensagem enviada para o servidor: %s\n", buffer);
  if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&(serveraddr), addrlen)==-1){
    printf("Error sending\n");
    exit(1);
  }
  if((n=recvfrom(socketfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&(serveraddr), &addrlen))==-1){
    printf("Error on recvfrom\n");
    exit(1);
  }
  buffer[n]='\0';
  printf("%s\n",buffer);
}


int main(int argc, char *argv[]){

	int i;
	char *name, *surname, 
	ip[128], scport[128], snpip[128], snpport[128], ipdooutro[128], portodooutro[128];

	int sair=1;
	
  fd_set rfds;
  int maxfd, counter;
  char buffer[128];
  int len;

  char cabecalho[128], parametros[128];

  struct sockaddr_in name_server;
  int name_socket;

	for(i=1; i<argc; i=i+2){

    switch(argv[i][1]){
      
      case 's':
      	strcpy(snpip, argv[i+1]);
      break;

      case 'n':
      	name=strtok(argv[i+1], ".");
      	surname=strtok(NULL, "\0");
      break;
      
      case 'q':
      	strcpy(snpport, argv[i+1]);
      break;
      
      case 'i':
      	strcpy(ip, argv[i+1]);
      break;
      
      case 'p':
      	strcpy(scport, argv[i+1]);
      break;
      
      default:
	      printf("Sem parametros. Saindo.\n");
	      exit(-1);
      break;
    }
  }
  printf("name: %s\nsurname: %s\nsnpip: %s\nsnpport: %s\nsaip: %s\nscport: %s\n", name, surname, snpip, snpport, ip, scport);

  name_socket = newudpclient(&name_server, snpip, IP, snpport);
  while(sair){

    FD_ZERO(&rfds);
    FD_SET(STDIN, &rfds);
    FD_SET(name_socket, &rfds);
    maxfd=name_socket;

    counter = select(maxfd+1, &rfds, NULL, NULL, NULL);

    if(counter<0){
      printf("Error on select\n");
      exit(4);
    }else if(counter>0){
	    if(FD_ISSET(STDIN, &rfds)){
	      fgets(buffer, sizeof(buffer), stdin);
	      len = strlen(buffer) - 1;
	      if (buffer[len] == '\n')
	        buffer[len] = '\0';
	      sscanf(buffer, "%s %s", cabecalho, parametros);
	      printf("Mensagem Recebida: %s\nCabecalho: %s\nParametros: %s\n", buffer, cabecalho, parametros);
	      if(strcmp(cabecalho, "join")==0){
	      	REG(name, surname, ip, scport, name_socket, name_server);

	      }else if(strcmp(cabecalho, "leave")==0){
	      	UNR(name, surname, name_socket, name_server);
	      	
	      }else if(strcmp(cabecalho, "find")==0){
	      	QRY(parametros, &ipdooutro, &portodooutro);
	      	
	      }else if(strcmp(cabecalho, "connect")==0){
	      	
	      }else if(strcmp(cabecalho, "message")==0){
	      	
	      }else if(strcmp(cabecalho, "disconnect")==0){
	      	
	      }else if(strcmp(cabecalho, "exit")==0){
	      	sair=0;
	      	
	      }else{
					printf("Cabecalho (%s) do comando introduzido nao reconhecido\n", cabecalho);
	      }
	    }
    }
  }

	return 0;
}
