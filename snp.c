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


typedef struct _localization {
        char name[64];
        char surname[64];
        char ip[64];
        char port[64];
} localization;

typedef struct _List {
        localization data;
        struct _List *next;
} List;

typedef struct _Row {
        List *first;
        List *last;
        int size;
} Row;

void init(Row *row){
        row->first=NULL;
        row->last=NULL;
        row->size=0;
}

List *newNode(localization x){
        List *new=calloc(1, sizeof(List));
        if(new==NULL) {
                printf("erro a alocar\n");
                exit(-1);
        }
        new->data = x;
        new->next = NULL;
        return new;
}

void add(Row *row, localization x){
        List *new;

        new=newNode(x);
        if(row->size==0) {
                row->first=new;
        }
        if(row->size==1) {
                row->last=new;
                row->first->next=new;
        }
        if(row->size>=2) {
                row->last->next=new;
                row->last=new;
        }
        row->size=(row->size)+1;
}

void printlist(Row row){
        List *aux;

        if(row.size==0) {
                printf("lista vazia\n");
        }

        if(row.size==1) {
                aux=row.first;
                printf("%s;%s;%s\n", (aux->data).name, (aux->data).ip, (aux->data).port);
        }

        if(row.size>=2) {
                for(aux=row.first; aux!=NULL; aux=aux->next) {
                        printf("%s;%s;%s\n", (aux->data).name, (aux->data).ip, (aux->data).port);
                }
        }
        return;
}

int searchList(Row row, localization *result, char *name){
        List *aux;

        for(aux=row.first; aux!=NULL; aux=aux->next) {
                if(strcmp((aux->data).name, name)==0) {
                        if(result!=NULL) {
                                strcpy(result->port, (aux->data).port);
                                strcpy(result->ip, (aux->data).ip);
                                strcpy(result->name, (aux->data).name);
                                strcpy(result->surname, (aux->data).surname);
                        }
                        return 1;
                }
        }
        return 0;
}

void removeList(Row *row, char *name){
        int i=0;
        List *aux, *aux1;

        if(row->size==1) {
                free(row->first);
                row->first = NULL;
        }

        if(row->size==2) {
                if(strcmp(((row->last)->data).name, name)==0) {
                        free(row->last);
                        row->last = NULL;
                        row->first->next=NULL;
                }
                else{
                        free(row->first);
                        row->first=row->last;
                        row->last = NULL;
                }
        }

        if(row->size>=3) {
                for(aux=row->first; aux!=NULL&&i==0; aux=aux->next) {
                        if(strcmp((aux->data).name, name)==0) {
                                if(aux==row->first) {
                                        aux1=aux->next;
                                        row->first=aux->next;
                                        free(aux);
                                        i++;
                                        aux=aux1;
                                }
                                else if(aux==row->last) {
                                        row->last=aux1;
                                        (row->last)->next=NULL;
                                        free(aux);
                                        i++;
                                        aux=aux1;
                                }
                                else{
                                        aux1->next=aux->next;
                                        free(aux);
                                        i++;
                                        aux=aux1;
                                }
                        }
                        else
                                aux1=aux;
                }
        }
        row->size=(row->size)-1;
}

int newudpserver(struct sockaddr_in *serveraddr, char port[128]){

        int fd;

        fd=socket(AF_INET, SOCK_DGRAM, 0);
        if(fd==-1) {
                printf("Error opening socket\n");
                exit(1);
        }

        memset((void*)&(*serveraddr),(int)'\0',sizeof((*serveraddr)));
        (*serveraddr).sin_family=AF_INET;
        (*serveraddr).sin_addr.s_addr=htonl(INADDR_ANY);
        (*serveraddr).sin_port=htons((u_short)atoi(port));

        return fd;
}

int newudpclient(struct sockaddr_in *serveraddr, char *name, int chartype, char *port){

        int fd;
        struct hostent *h;

        fd=socket(AF_INET, SOCK_DGRAM, 0);
        if(fd==-1) {
                printf("Error opening socket\n");
                exit(1);
        }

        memset((void*)&(*serveraddr),(int)'\0',sizeof((*serveraddr)));
        (*serveraddr).sin_family=AF_INET;

        if(chartype == NAME) {
                if((h=gethostbyname(name))==NULL) {
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

int sendProtocolMessage(char *buffer, int socketfd, struct sockaddr_in serveraddr){

        unsigned int addrlen;
        int n;

        int counter;
        struct timeval tv;
        fd_set funcfds;
        int i;
        int sent=0;

        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&funcfds);
        FD_SET(socketfd, &funcfds);

        addrlen = sizeof((serveraddr));

        char msgReceived[128];

        for( i = 0; i < 2; i++) {
                if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&(serveraddr), addrlen)==-1) {
                        printf("Error sending\n");
                        exit(1);
                }
                counter = select(socketfd+1, &funcfds, NULL, NULL, &tv);
                if(counter<0) {
                        printf("Error on select\n");
                        exit(4);
                }else if(counter>0) {
                        if((n=recvfrom(socketfd, msgReceived, sizeof(msgReceived), 0, (struct sockaddr*)&(serveraddr), &addrlen))==-1) {
                                printf("Error on recvfrom\n");
                                exit(1);
                        }
                        msgReceived[n]='\0';
                        printf("%s\n",msgReceived);
                        i=3;
                        sent=1;
                }
        }
        if(!sent) {
                printf("Could not send message. Try again later...\n");
        }
        strcpy(buffer, msgReceived);
        return sent;
}

int SREG(char surname[128], char ip[128], char port[128], int socketfd, struct sockaddr_in serveraddr){
        char *buffer;
        char cabecalho[128];
        buffer = calloc(128, sizeof(char));

        sprintf(buffer, "SREG %s;%s;%s", surname, ip, port);
        printf("%s\n", buffer);
        sendProtocolMessage(buffer, socketfd, serveraddr);
        sscanf(buffer, "%s ", cabecalho);
        if(strcmp(cabecalho, "OK")!=0&&strcmp(cabecalho, "NOK")!=0) {
                printf("Could not decipher message received, you should try again...\n");
                return 0;
        }

        free(buffer);
        return 1;
}

int SUNR(char surname[128], int socketfd, struct sockaddr_in serveraddr){
        char *buffer;
        char cabecalho[128];
        buffer = calloc(128, sizeof(char));

        sprintf(buffer, "SUNR %s", surname);
        printf("%s\n", buffer);
        sendProtocolMessage(buffer, socketfd, serveraddr);
        sscanf(buffer, "%s ", cabecalho);
        if(strcmp(cabecalho, "OK")!=0&&strcmp(cabecalho, "NOK")!=0) {
                printf("Could not decipher message received, you should try again...\n");
                return 0;
        }

        free(buffer);
        return 1;
}

void REG(char parametros[128], Row *row, int socketfd, struct sockaddr_in serveraddr){
        localization ipport;
        char *name, *surname, *ip, *port;
        char buffer[128];
        unsigned int addrlen;

        name=strtok(parametros, ".");
        surname=strtok(NULL, ";");
        ip=strtok(NULL, ";");
        port=strtok(NULL, "\0");

        if(searchList(*row, NULL,name)) {
                addrlen = sizeof(serveraddr);
                sprintf(buffer, "NOK Name already registered\n");
                printf("%s\n", buffer);
                if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&serveraddr, addrlen)==-1) {
                        printf("Error sending\n");
                        exit(1);
                }
        }else{

                strcpy(ipport.name, name);
                strcpy(ipport.surname, surname);
                strcpy(ipport.ip, ip);
                strcpy(ipport.port, port);


                add(row, ipport);

                addrlen = sizeof(serveraddr);
                sprintf(buffer, "OK\n");
                printf("%s\n", buffer);
                if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&serveraddr, addrlen)==-1) {
                        printf("Error sending\n");
                        exit(1);
                }
        }
}

void UNR(char parametros[128], Row *row, int socketfd, struct sockaddr_in serveraddr){
        char *name;
        char buffer[128];
        unsigned int addrlen;

        name=strtok(parametros, ".");

        if(!searchList(*row, NULL,name)) {
                addrlen = sizeof(serveraddr);
                sprintf(buffer, "NOK Name not registered\n");
                printf("%s\n", buffer);
                if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&serveraddr, addrlen)==-1) {
                        printf("Error sending\n");
                        exit(1);
                }
                return;
        }else{

                removeList(row, name);

                addrlen = sizeof(serveraddr);
                sprintf(buffer, "OK\n");
                printf("%s\n", buffer);
                if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&serveraddr, addrlen)==-1) {
                        printf("Error sending\n");
                        exit(1);
                }
        }
}

void SRPL(char name[128], char parametros[128], int me_socket, struct sockaddr_in me_server){
        char *surname, *snpip, *snpport;
        int contactsnp_socket;
        struct sockaddr_in contactsnp_server;

        unsigned int addrlen;
        char *buffer;
        buffer = calloc(128, sizeof(char));
        char warning[128];

        int flag=0;

        surname=strtok(parametros, ";");
        snpip=strtok(NULL, ";");
        snpport=strtok(NULL, "\0");

        sprintf(buffer, "QRY %s.%s", name, surname);
        printf("%s\n", buffer);

        contactsnp_socket = newudpclient(&contactsnp_server, snpip, IP, snpport);

        flag=sendProtocolMessage(buffer, contactsnp_socket, contactsnp_server);

        if(!flag) {
                addrlen = sizeof(me_server);
                sprintf(warning, "NOK - Could not reach name server surname server gave...\n");
                printf("%s\n", warning);
                if(sendto(me_socket, warning, strlen(warning)+1, 0, (struct sockaddr*)&me_server, addrlen)==-1) {
                        printf("Error sending\n");
                        exit(1);
                }
        }else{
                addrlen = sizeof(me_server);
                printf("%s\n", buffer);
                if(sendto(me_socket, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&me_server, addrlen)==-1) {
                        printf("Error sending\n");
                        exit(1);
                }
        }
        free(buffer);
}

void SQRY(char name[128], char surname[128], int surname_socket, struct sockaddr_in surname_server, int me_socket, struct sockaddr_in me_server){
        char *buffer;
        buffer = calloc(128, sizeof(char));

        char cabecalho[128], parametros[128];

        unsigned int addrlen;
        char warning[128];

        sprintf(buffer, "SQRY %s", surname);
        printf("%s\n", buffer);
        if(!sendProtocolMessage(buffer, surname_socket, surname_server)) {
                addrlen = sizeof(me_server);
                sprintf(warning, "NOK - Could not reach surname server...\n");
                printf("%s\n", warning);
                if(sendto(me_socket, warning, strlen(warning)+1, 0, (struct sockaddr*)&me_server, addrlen)==-1) {
                        printf("Error sending\n");
                        exit(1);
                }
        }else{
                if(strcmp(buffer, "SRPL")==0) {
                        addrlen = sizeof(me_server);
                        sprintf(warning, "RPL");
                        printf("%s\n", warning);
                        if(sendto(me_socket, warning, strlen(warning)+1, 0, (struct sockaddr*)&me_server, addrlen)==-1) {
                                printf("Error sending\n");
                                exit(1);
                        }
                }else{
                        sscanf(buffer, "%s %s", cabecalho, parametros);
                        SRPL(name, parametros, me_socket, me_server);
                }
        }
        free(buffer);
}

void QRY(char me_surname[128], char parametros[128], int surname_socket, struct sockaddr_in surname_server, int me_socket, struct sockaddr_in me_server, Row row){
        char *name, *surname;
        char buffer[128];
        unsigned int addrlen;
        localization info;
        name=strtok(parametros, ".");
        surname=strtok(NULL, "\0");

        if(strcmp(me_surname, surname)==0) {
                if(searchList(row, &info, name)) {
                        addrlen = sizeof(me_server);
                        sprintf(buffer, "RPL %s.%s;%s;%s", info.name, info.surname, info.ip, info.port);
                        printf("%s\n", buffer);
                        if(sendto(me_socket, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&me_server, addrlen)==-1) {
                                printf("Error sending\n");
                                exit(1);
                        }
                }else{
                        addrlen = sizeof(me_server);
                        sprintf(buffer, "RPL");
                        printf("%s\n", buffer);
                        if(sendto(me_socket, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&me_server, addrlen)==-1) {
                                printf("Error sending\n");
                                exit(1);
                        }
                }
        }else{
                SQRY(name, surname, surname_socket, surname_server, me_socket, me_server);
        }
}

int main(int argc, char *argv[]){

        int i;
        char surname[128], snpip[128], snpport[128], saip[128], saport[128];
        char cabecalho[128], parametros[128];
        int me_socket, surname_socket;
        int sair=1;
        struct sockaddr_in me_server, surname_server;

        fd_set rfds;
        int maxfd, counter;
        char buffer[128];
        int len;
        unsigned addrlen;

        Row row;
        init(&row);

        printf("%d\n",argc);

        if(argc!=7 && argc!=11) {
                printf("Something went wrong...\nUsage: schat -n <surname> -s <ip> -q <port> -i <saip> -p <saport>\n");
                exit(4);
        }else{
                for(i=1; i<argc; i=i+2) {

                        switch(argv[i][1]) {

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
                                printf("Something went wrong...\nUsage: schat -n <surname> -s <ip> -q <port> -i <saip> -p <saport>\n");
                                exit(-1);
                                break;
                        }
                }
        }

        int type = IP;
        if(argc!=11) {
                strcpy(saport, "58000");
                strcpy(saip, "tejo.tecnico.ulisboa.pt");
                type = NAME;
        }

        surname_socket = newudpclient(&surname_server, saip, type, saport);

        if(!SREG(surname,snpip,snpport,surname_socket,surname_server))
                exit(1);

        me_socket=newudpserver(&me_server, snpport);


        if(bind(me_socket, (struct sockaddr*)&me_server, sizeof(me_server)) < 0) {
                printf("Error on binding\n");
                exit(1);
        }

        while(sair) {
                FD_ZERO(&rfds);
                FD_SET(STDIN, &rfds);
                FD_SET(surname_socket, &rfds);
                FD_SET(me_socket, &rfds);
                maxfd=me_socket;

                counter = select(maxfd+1, &rfds, NULL, NULL, NULL);

                if(counter<0) {
                        printf("Error on select\n");
                        exit(4);
                }else if(counter>0) {
                        if(FD_ISSET(STDIN, &rfds)) {
                                fgets(buffer, sizeof(buffer), stdin);
                                /* Remove trailing newline character from the input buffer if needed. */
                                len = strlen(buffer) - 1;
                                if (buffer[len] == '\n')
                                        buffer[len] = '\0';

                                if(strcmp(buffer, "exit")==0) {
                                        if(SUNR(surname, surname_socket, surname_server)) {
                                                sair=0;
                                                close(me_socket);
                                                close(surname_socket);
                                        }
                                }else if(strcmp(buffer, "list")==0) {
                                        printlist(row);
                                }else if(strcmp(buffer, "clear")==0) {
                                        printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
                                }
                        }else{

                                if(FD_ISSET(me_socket, &rfds)) {
                                        addrlen = sizeof(me_server);
                                        if((len=recvfrom(me_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&me_server, &addrlen))==-1) {
                                                printf("Error on recvfrom on socket %d\n", surname_socket);
                                                exit(1);
                                        }
                                } if(FD_ISSET(surname_socket, &rfds)) {
                                        addrlen = sizeof(surname_server);
                                        if((len=recvfrom(surname_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&surname_server, &addrlen))==-1) {
                                                printf("Error on recvfrom on socket %d\n", surname_socket);
                                                exit(1);
                                        }
                                }

                                buffer[len]='\0';
                                sscanf(buffer, "%s %s", cabecalho, parametros);
                                printf("%s\n", buffer);
                                switch(cabecalho[0]) {
                                case 'R':
                                        REG(parametros, &row, me_socket, me_server);
                                        break;

                                case 'U':
                                        UNR(parametros, &row, me_socket, me_server);
                                        break;

                                case 'Q':
                                        QRY(surname, parametros, surname_socket, surname_server, me_socket, me_server, row);
                                        break;

                                default:
                                        printf("Header (%s) unknown\n", cabecalho);
                                        break;
                                }
                        }
                }
        }
}
