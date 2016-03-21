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
#include <time.h>

#define PORT 58000
#define STDIN 0

#define NAME 0
#define IP 1

int newtcpserver(struct sockaddr_in *serveraddr, char port[128]){
  int fd;

  fd=socket(AF_INET, SOCK_STREAM, 0);
  if(fd==-1){
    printf("Error opening socket\n");
    exit(1);
  }

  memset((void*)&(*serveraddr),(int)'\0',sizeof((*serveraddr)));
  (*serveraddr).sin_family=AF_INET;
  (*serveraddr).sin_addr.s_addr=htonl(INADDR_ANY);
  (*serveraddr).sin_port=htons((u_short)atoi(port));

  return fd;
}

int newtcpclient(struct sockaddr_in *serveraddr, char *ip, char *port){

  int fd;

  fd=socket(AF_INET, SOCK_STREAM, 0);
  if(fd==-1){
    printf("Error opening socket\n");
    exit(1);
  }

  memset((void*)&(*serveraddr),(int)'\0',sizeof((*serveraddr)));
  (*serveraddr).sin_family=AF_INET;
  inet_pton(AF_INET, ip, &((*serveraddr).sin_addr));
  (*serveraddr).sin_port=htons((u_short)atoi(port));
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
  (*serveraddr).sin_port=htons((u_short)atoi(port));
  return fd;
}

int sendProtocolMessage(char *buffer, int socketfd, struct sockaddr_in serveraddr){

  int addrlen;
  int n;

  int counter;
  struct timeval tv;
  fd_set funcfds;
  int i;
  int sent=0;

  tv.tv_sec = 10;
  tv.tv_usec = 0;
  FD_ZERO(&funcfds);
  FD_SET(socketfd, &funcfds);

  addrlen = sizeof((serveraddr));

  char msgReceived[128];

  for( i = 0; i < 1; i++){
    if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&(serveraddr), addrlen)==-1){
      printf("Error sending\n");
      exit(1);
    }
    counter = select(socketfd+1, &funcfds, NULL, NULL, &tv);
    if(counter<0){
      printf("Error on select\n");
      exit(4);
    }else if(counter>0){
      if((n=recvfrom(socketfd, msgReceived, sizeof(msgReceived), 0, (struct sockaddr*)&(serveraddr), &addrlen))==-1){
        printf("Error on recvfrom\n");
        exit(1);
      }
      msgReceived[n]='\0';
      printf("%s\n",msgReceived);
      i=3;
      sent=1;
    }else{
      printf("Erro no envio da mensagem, a tentar novamente...(tentativa %d)\n", i+1);
    }
  }
  if(!sent){
    printf("Não foi possivel o envio da mensagem, tente novamente mais tarde.\n");
  }
  strcpy(buffer, msgReceived);
  return sent;
}

void REG(char name[128], char surname[128], char ip[128], char scport[128], int socketfd, struct sockaddr_in serveraddr){
  char *buffer;
  buffer = calloc(128, sizeof(char));

  sprintf(buffer, "REG %s.%s;%s;%s", name, surname, ip, scport);
  printf("Mensagem enviada: %s\n", buffer);
  sendProtocolMessage(buffer, socketfd, serveraddr);
}

void UNR(char name[128], char surname[128], int socketfd, struct sockaddr_in serveraddr){

  char *buffer;

  buffer = calloc(128, sizeof(char));

  sprintf(buffer, "UNR %s.%s", name, surname);
  printf("Mensagem enviada: %s\n", buffer);
  sendProtocolMessage(buffer, socketfd, serveraddr);
}

int QRY(char parametros[128], int socketfd, struct sockaddr_in serveraddr, char *contactip, char *contactport){
  char *buffer, *port, *ip, cabecalho[128], parametroos[128];
  buffer = calloc(128, sizeof(char));

  sscanf(parametros, "%s", parametroos);
  sprintf(buffer, "QRY %s", parametroos);
  printf("Mensagem enviada: %s\n", buffer);
  sendProtocolMessage(buffer, socketfd, serveraddr);
  sscanf(buffer, "%s ", cabecalho);
  if(strcmp(buffer, "RPL")==0){
    printf("O cliente que deseja contactar não se encontra registado\n");
    return 0;
  }if (strcmp(cabecalho, "NOK")==0) {
    return 0;
  }
  strtok(buffer, ";");
  ip=strtok(NULL, ";");
  port=strtok(NULL, "\0");
  strcpy(contactip, ip);
  strcpy(contactport, port);
  return 1;
}

int main(int argc, char *argv[]){

  int i, k=0, line;
  char *reader, name[128], surname[128], contactname[128], contactsurname[128],
  ip[128], scport[128], snpip[128], snpport[128], file[128], fileadd[128], byte[128];

  char *contactip, *contactport;
  contactport = calloc(128, sizeof(char));
  contactip = calloc(128, sizeof(char));

  int sair=1;

  FILE *fp;

  fd_set rfds;
  int maxfd, counter;
  char buffer[128];
  int len;

  char cabecalho[128], parametros[128], char_line[128];

  struct sockaddr_in name_server, me_server, contact_server;
  int name_socket, me_socket, contact_socket;
  int contact_flag = 0, me_flag = 0;
  int addrlen;

  int auth_sent, auth_recv, auth_file;


  if(argc!=11){
    printf("Algo errado com os parametros colocados, por favor reveja-os.\n");
    exit(4);
  }else{
    for(i=1; i<argc; i=i+2){

      switch(argv[i][1]){

        case 's':
          strcpy(snpip, argv[i+1]);
        break;

        case 'n':
          reader=strtok(argv[i+1], ".");
          strcpy(name, reader);
          reader=strtok(NULL, "\0");
          strcpy(surname, reader);

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
  }
  printf("name: %s\nsurname: %s\nsnpip: %s\nsnpport: %s\nsaip: %s\nscport: %s\n", name, surname, snpip, snpport, ip, scport);

  name_socket= newudpclient(&name_server, snpip, IP, snpport);
  printf("name_socket = %d\n", name_socket);

  me_socket= newtcpserver(&me_server, scport);
  printf("me_socket = %d\n", me_socket);

  if(bind(me_socket, (struct sockaddr*)&me_server, sizeof(me_server)) < 0){
    printf("Error on binding\n");
    exit(1);
  }
  if(listen(me_socket,2)==-1){
    printf("Error on listening\n");
    exit(1);
  }

  while(sair){

    FD_ZERO(&rfds);
    FD_SET(STDIN, &rfds);
    maxfd = STDIN;
    if(contact_flag){
      FD_SET(contact_socket, &rfds);
      if(contact_socket>maxfd)
        maxfd = contact_socket;
    }if(me_flag){
      FD_SET(me_socket, &rfds);
      if(me_socket>maxfd)
        maxfd = me_socket;
    }

    counter = select(maxfd+1, &rfds, NULL, NULL, NULL);

    if(counter<0){
      printf("Error on select\n");
      exit(4);
    }else if(counter>0){

      if(FD_ISSET(STDIN, &rfds)){
        fgets(buffer, sizeof(buffer), stdin);
        len = strlen(buffer) - 1;

        if(buffer[len] == '\n')
          buffer[len] = '\0';

        sscanf(buffer, "%s %[^\t\n]", cabecalho, parametros);
        printf("Comando: %s\nCabecalho: %s\nParametros: %s\n", buffer, cabecalho, parametros);

        if(strcmp(cabecalho, "join")==0){

          REG(name, surname, ip, scport, name_socket, name_server);
          me_flag = 1;

        }else if(strcmp(cabecalho, "leave")==0){

          UNR(name, surname, name_socket, name_server);
          me_flag = 0;

        }else if(strcmp(cabecalho, "find")==0){
          QRY(parametros, name_socket, name_server, contactip, contactport);

        }else if(strcmp(cabecalho, "connect")==0){
          if(!QRY(parametros, name_socket, name_server, contactip, contactport))
            printf("Nao foi posssivel encontrar o utilizador na rede.\n");
          else{

      			sscanf(parametros, "%s %s", cabecalho, file);
            reader = strtok(cabecalho, ".");
            strcpy(contactname, reader);
            reader = strtok(NULL, "\0");
            strcpy(contactsurname, reader);
            printf("Name: %s\nSurname: %s\n", contactname, contactsurname);

      			sprintf(fileadd,"keyfile/%s.txt", file);

            fp = fopen (fileadd, "r");
            if(fp==NULL){
              printf("Error opening keyfile\n");
              exit(1);
            }

            contact_socket = newtcpclient(&contact_server, contactip, contactport);
            contact_flag=1;
            if(connect(contact_socket,(struct sockaddr*)&contact_server, sizeof(contact_server)) < 0){
              printf("Error connecting\n");
              close(contact_socket);
              contact_flag=0;
            }

            sprintf(buffer, "NAME %s.%s", name, surname);
            if(write(contact_socket, buffer, sizeof(buffer))==-1){
              printf("Error on writing\n");
              exit(1);
            }
            len=read(contact_socket, buffer, sizeof(buffer));
            if(len==-1){
              printf("Error on reading\n");
              exit(1);
            }if(len==0) {
              printf("Other client closed connection.\n");
              contact_flag=0;
            }else{
              buffer[len] = '\0';
              printf("Mensagem recebida: %s\n", buffer);
            }
            sscanf(buffer, "%s %s", cabecalho, char_line);
            line = atoi(char_line) - 1;

            while ((fgets(byte, sizeof(byte), fp) != NULL )&&(k<line)){
      		    k++;
      			}

            sprintf(buffer, "AUTH %s", byte);
            if(write(contact_socket, buffer, sizeof(buffer))==-1){
              printf("Error on writing\n");
              exit(1);
            }
/*************************************/
            srand(time(NULL));
            auth_sent = rand() % 256;
            printf("Sending random byte number %d.\n", auth_sent);
            sprintf(buffer, "AUTH %d", auth_sent);
            if(write(contact_socket, buffer, sizeof(buffer))==-1){
              printf("Error on writing\n");
              exit(1);
            }

            len=read(contact_socket, buffer, sizeof(buffer));
            if(len==-1){
              printf("Error on reading\n");
              exit(1);
            }if(len==0) {
              printf("Other client closed connection.\n");
              contact_flag=0;
            }else{
              buffer[len] = '\0';
              printf("Mensagem recebida: %s\n", buffer);
            }

            sscanf(buffer, "%s %s", cabecalho, parametros);

            auth_recv = atoi(parametros);

            line = auth_sent - 1;
            fp = fopen("keyfile/keyfile1.txt", "r");
            if(fp==NULL){
              printf("Error opening keyfile\n");
              exit(1);
            }


            while ((fgets(byte, sizeof(byte), fp) != NULL )&&(k<line))
              k++;

            auth_file = atoi(byte);

            printf("auth_file = %d\nauth_recv = %d\n", auth_file, auth_recv);

            if(auth_file != auth_recv){
              printf("Could not authenticate %s %s, closing connection...\n",contactname, contactsurname);
              close(contact_socket);
              contact_flag = 0;
            }

            fclose(fp);
          }

        }else if(strcmp(cabecalho, "disconnect")==0){
          close(contact_socket);
          contact_flag=0;

        }else if(strcmp(cabecalho, "message")==0){
          sprintf(buffer, "%s", parametros);

      	  if(write(contact_socket, buffer, sizeof(buffer))==-1){
      		  printf("Error on writing\n");
      		  exit(1);
      	  }

        }else if(strcmp(cabecalho, "exit")==0){
          sair=0;
        }else if(strcmp(cabecalho, "clear")==0){
          printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        }else{
          printf("Cabecalho (%s) do comando introduzido nao reconhecido\n", cabecalho);
        }
      }if(me_flag){
        if(FD_ISSET(me_socket, &rfds)){
          addrlen = sizeof(me_server);
      	  if((contact_socket=accept(me_socket,(struct sockaddr *)&me_server,  &addrlen))==-1){
      		  printf("Error on accepting\n");
      		  exit(1);
      	  }
          contact_flag = 1;
          printf("Someone just connected to you... trying to authenticate.\n");

          len=read(contact_socket, buffer, sizeof(buffer));
          if(len==-1){
            printf("Error on reading\n");
            exit(1);
          }if(len==0) {
            printf("Other client closed connection.\n");
            contact_flag=0;
          }else{
            buffer[len] = '\0';
            printf("Mensagem recebida: %s\n", buffer);
          }

          sscanf(buffer, "%s %s", cabecalho, parametros);
          reader = strtok(parametros, ".");
          strcpy(contactname, reader);
          reader = strtok(NULL, "\0");
          strcpy(contactsurname, reader);
          printf("Name: %s\nSurname: %s\n", contactname, contactsurname);

          srand(time(NULL));
          auth_sent = rand() % 256;
          printf("Sending random byte number %d.\n", auth_sent);
          sprintf(buffer, "AUTH %d", auth_sent);
          if(write(contact_socket, buffer, sizeof(buffer))==-1){
            printf("Error on writing\n");
            exit(1);
          }

          len=read(contact_socket, buffer, sizeof(buffer));
          if(len==-1){
            printf("Error on reading\n");
            exit(1);
          }if(len==0) {
            printf("Other client closed connection.\n");
            contact_flag=0;
          }else{
            buffer[len] = '\0';
            printf("Mensagem recebida: %s\n", buffer);
          }

          sscanf(buffer, "%s %s", cabecalho, parametros);

          auth_recv = atoi(parametros);

          line = auth_sent - 1;
          fp = fopen("keyfile/keyfile1.txt", "r");
          if(fp==NULL){
            printf("Error opening keyfile\n");
            exit(1);
          }


          while ((fgets(byte, sizeof(byte), fp) != NULL )&&(k<line))
            k++;

          auth_file = atoi(byte);

          printf("auth_file = %d\nauth_recv = %d\n", auth_file, auth_recv);

          if(auth_file != auth_recv){
            printf("Could not authenticate %s %s, closing connection...\n",contactname, contactsurname);
            close(contact_socket);
            contact_flag = 0;
          }
/*************************************************/
          len=read(contact_socket, buffer, sizeof(buffer));
          if(len==-1){
            printf("Error on reading\n");
            exit(1);
          }if(len==0) {
            printf("Other client closed connection.\n");
            contact_flag=0;
          }else{
            buffer[len] = '\0';
            printf("Mensagem recebida: %s\n", buffer);
          }
          sscanf(buffer, "%s %s", cabecalho, char_line);
          line = atoi(char_line) - 1;

          while ((fgets(byte, sizeof(byte), fp) != NULL )&&(k<line)){
            k++;
          }

          sprintf(buffer, "AUTH %s", byte);
          if(write(contact_socket, buffer, sizeof(buffer))==-1){
            printf("Error on writing\n");
            exit(1);
          }
          fclose(fp);
        }
      }if(contact_flag){
        if(FD_ISSET(contact_socket, &rfds)){

          len=read(contact_socket, buffer, sizeof(buffer));
          if(len==-1){
            printf("Error on reading\n");
            exit(1);
          }if(len==0){
            printf("Other client closed connection.\n");
            contact_flag=0;
          }else{
            buffer[len] = '\0';
            printf("%s %s: %s\n", contactname, contactsurname, buffer);
          }
        }
      }
    }
  }

  close(me_socket);
  close(name_socket);

  return 0;
}
