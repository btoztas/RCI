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

int searchDatabase(){
  return 0;
}

void printDatabase(FILE *database){
  char *buffer;
  while(fscanf(database, "%s", buffer)>0);
    printf("Read Buffer: %s\n", buffer);
}

FILE * addDatabase(FILE *database, char *name, char *surname, char *ip, char *port){
  FILE *aux;

  fclose(database);
  aux=fopen(surname, "w");
  fprintf(aux, "%s;%s;%s\n", name, ip, port);
  fclose(aux);
  return fopen(surname, "r");
}

int newudpserver(struct sockaddr_in *serveraddr){

  int fd;

  fd=socket(AF_INET, SOCK_DGRAM, 0);
  if(fd==-1){
    printf("Error opening socket\n");
    exit(1);
  }

  memset((void*)&(*serveraddr),(int)'\0',sizeof((*serveraddr)));
  (*serveraddr).sin_family=AF_INET;
  (*serveraddr).sin_addr.s_addr=htonl(INADDR_ANY);
  (*serveraddr).sin_port=htons((u_short)PORT);

  return fd;
}

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
  }
  (*serveraddr).sin_port=htons((u_short)atoi(port)); /*PORTA?*/
  return fd;
}

void SREG(char surname[128], char ip[128], char port[128], int socketfd, struct sockaddr_in serveraddr){
  
  int n;
  int addrlen;
  char buffer[128];

  addrlen = sizeof((serveraddr));
  sprintf(buffer, "SREG %s;%s;%s", surname, ip, port);
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

void SUNR(char surname[128], int socketfd, struct sockaddr_in serveraddr){
  
  int n;
  int addrlen;
  char buffer[128];
  
  addrlen = sizeof(serveraddr);
  sprintf(buffer, "SUNR %s", surname);
  printf("Mensagem enviada para o servidor: %s\n", buffer);
  if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&serveraddr, addrlen)==-1){
    printf("Error sending\n");
    exit(1);
  }
  if((n=recvfrom(socketfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&serveraddr, &addrlen))==-1){
    printf("Error on recvfrom\n");
    exit(1);
  }
  buffer[n]='\0';
  printf("%s\n",buffer);
}

void REG(char parametros[128]){
  char *name, *surname, *ip, *port;

  name=strtok(parametros, ".");
  surname=strtok(NULL, ";");
  ip=strtok(NULL, ";");
  port=strtok(NULL, "\0");

}

void UNR(char parametros[128]){
  char *name, *surname;

  name=strtok(parametros, ".");
  surname=strtok(NULL, "\0");
}

void SRPL(char parametros[128]){
  char *surname, *snpip, *scport;

  surname=strtok(parametros, ";");
  snpip=strtok(NULL, ";");
  scport=strtok(NULL, "\0");
}

void QRY(char parametros[128]){
  char *name, *surname;

  name=strtok(parametros, ".");
  surname=strtok(NULL, "\0");
}

void RPL(char parametros[128]){
  char *name, *surname, *scip, *scport;

  name=strtok(parametros, ".");
  surname=strtok(NULL, ";");
  scip=strtok(NULL, ";");
  scport=strtok(NULL, "\0");
}
  
int SQRY(char surname[128], int socketfd, struct sockaddr_in serveraddr){
  int n;
  int addrlen;
  char buffer[128];
  
  addrlen = sizeof(serveraddr);
  sprintf(buffer, "SQRY %s", surname);
  printf("Mensagem enviada para o servidor: %s\n", buffer);
  if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&serveraddr, addrlen)==-1){
    printf("Error sending\n");
    exit(1);
  }
}

int main(int argc, char *argv[]){
  
  int i;
  char surname[128], snpip[128], snpport[128], saip[128], saport[128], al[64];
  char cabecalho[128], parametros[128];
  int me_socket, addrlen, surname_socket;
  int sair=1;
  struct sockaddr_in serveraddr, surname_server;

  fd_set rfds;
  int maxfd, counter;
  char buffer[128];
  int len;

  FILE *database;

  for(i=1; i<argc; i=i+2){
    
    switch(argv[i][1]){
     
      case 'n':
        strcpy(surname, argv[i+1]);
      break;
      
      case 's':
        strcpy(snpip, argv[i+1]);
      break;
      
      case 'q':
        strcpy(snpport, argv[i+1]);
      break;
      
      case 'i':
        strcpy(saip, argv[i+1]);
      break;
      
      case 'p':
        strcpy(saport, argv[i+1]);
      break;
      
      default:
        printf("Sem parametros. Saindo.\n");
        exit(-1);
      break;
    }
  }
  
  printf("surname: %s\nsnpip: %s\nsnpport: %s\nsaip: %s\nsaport: %s\n", surname, snpip, snpport, saip, saport);
  
  surname_socket = newudpclient(&surname_server, "tejo.tecnico.ulisboa.pt", NAME, "58000");
  SREG(surname,snpip,snpport,surname_socket,surname_server);

  me_socket=newudpserver(&serveraddr);
   
  
  if(bind(me_socket, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
    printf("Error on binding\n");
    exit(1);
  }

  database = fopen(surname, "w");
  if(database == NULL) {
    printf("Open error of input file\n");
    exit(1);
  }

  while(sair){

    FD_ZERO(&rfds);
    FD_SET(STDIN, &rfds);
    FD_SET(surname_socket, &rfds);
    FD_SET(me_socket, &rfds);
    maxfd=me_socket;

    counter = select(maxfd+1, &rfds, NULL, NULL, NULL);

    if(counter<0){
      printf("Error on select\n");
      exit(4);
    }else if(counter>0){
      if(FD_ISSET(STDIN, &rfds)){
        fgets(buffer, sizeof(buffer), stdin);
        /* Remove trailing newline character from the input buffer if needed. */
        len = strlen(buffer) - 1;
        if (buffer[len] == '\n')
          buffer[len] = '\0';
        printf("Mensagem: %s\n",buffer);
        
        if(strcmp(buffer, "exit")==0){
          sair=0;
          SUNR(surname, surname_socket, surname_server);
          close(me_socket);
          close(surname_socket);
          /*Enviar mensagem de UNRG*/
        }
        if(strcmp(buffer, "list")==0){
          printDatabase(database);
        }
      }else{

        if(FD_ISSET(me_socket, &rfds)){
          addrlen = sizeof(serveraddr);
          if((len=recvfrom(me_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&serveraddr, &addrlen))==-1){
            printf("Error on recvfrom on socket %d\n", surname_socket);
            exit(1);
          }
        }if(FD_ISSET(surname_socket, &rfds)){
          addrlen = sizeof(surname_server);
          if((len=recvfrom(surname_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&surname_server, &addrlen))==-1){
            printf("Error on recvfrom on socket %d\n", surname_socket);
            exit(1);
          }
        }

        buffer[len]='\0';
        sscanf(buffer, "%s %s", cabecalho, parametros);
        printf("Mensagem Recebida: %s\nCabecalho: %s\nParametros: %s\n", buffer, cabecalho, parametros);
        switch(cabecalho[0]){
          case 'R':
            switch(cabecalho[1]){
              case 'E':
                REG(parametros);
              break;

              case 'P':
                RPL(parametros);
              break;

              default:
                printf("Cabecalho (%s) da mensagem nao reconhecido\n", cabecalho);
              break;
            }

          case 'U':
            UNR(parametros);
          break;

          case 'S':
            SRPL(parametros);
          break;

          case 'Q':
            QRY(parametros);
          break;

          default:
            printf("Cabecalho (%s) do comando introduzido nao reconhecido\n", cabecalho);
          break;
        }
      }
    }
  } 
}