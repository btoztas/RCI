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
#include <signal.h>


#define STDIN 0


#define NAME 0
#define IP 1



/*
 *  Function:
 *    newtcpserver
 *
 *  Description:
 *    Creates a new tcp server socket
 *
 *  Arguments:
 *    struct sockaddr_in to put server characteristics
 *    port number where server is available
 *
 *
 *  Return value:
 *    file decriptor with socket number
 */

int newtcpserver(struct sockaddr_in *serveraddr, char port[128]){
        int fd;

        fd=socket(AF_INET, SOCK_STREAM, 0);         /*SOCK_STREM since it is a tcp connection */
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

/*
 *  Function:
 *    newtcpclient
 *
 *  Description:
 *    Creates a new tcp client socket
 *
 *  Arguments:
 *    struct sockaddr_in to put server characteristics
 *    port number where server is available
 *    ip where server is located
 *
 *
 *
 *  Return value:
 *    file decriptor with socket number
 */

int newtcpclient(struct sockaddr_in *serveraddr, char *ip, char *port){

        int fd;

        fd=socket(AF_INET, SOCK_STREAM, 0);      /*SOCK_STREM since it is a tcp connection */
        if(fd==-1) {
                printf("Error opening socket\n");
                exit(1);
        }

        memset((void*)&(*serveraddr),(int)'\0',sizeof((*serveraddr)));
        (*serveraddr).sin_family=AF_INET;
        inet_pton(AF_INET, ip, &((*serveraddr).sin_addr));
        (*serveraddr).sin_port=htons((u_short)atoi(port));
        return fd;
}

/*
 *  Function:
 *    newudpclient
 *
 *  Description:
 *    Creates a new udp client socket
 *
 *  Arguments:
 *    struct sockaddr_in to put server characteristics
 *    port number where server is available
 *    ip/name of the server
 *    flag (chartype) to distinct between ip or name
 *
 *
 *
 *  Return value:
 *    file decriptor with socket number
 */

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

        if(chartype == NAME) {                            /*if NAME, we need to  get the host */
                if((h=gethostbyname(name))==NULL) {
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


/*
 *  Function:
 *    sendProtocolMessage
 *
 *  Description:
 *    sends a new message to a server and receives the server feedback
 *
 *  Arguments:
 *    message to send
 *    server socket
 *    charecteristics of the server
 *
 *
 *
 *  Return value:
 *    0/1 in case of Error/success
 *    sprints to buffer feedback from server
 *
 */

int sendProtocolMessage(char *buffer, int socketfd, struct sockaddr_in serveraddr){

        unsigned int addrlen;
        int n;

        int counter;
        struct timeval tv;
        fd_set funcfds;
        int i;
        int sent=0;

        tv.tv_sec = 3;        /* 3 seconds timeout for the message */
        tv.tv_usec = 0;
        FD_ZERO(&funcfds);
        FD_SET(socketfd, &funcfds);

        addrlen = sizeof((serveraddr));

        char msgReceived[128];

        for( i = 0; i < 2; i++) { /* Tries to send the message twice*/
                if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&(serveraddr), addrlen)==-1) {
                        printf("Error sending\n");
                        exit(1);
                }
                counter = select(socketfd+1, &funcfds, NULL, NULL, &tv); /* select to implement message timeout */
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
                        sent=1; /* message sent with success */
                }
        }
        if(!sent) {
                printf("Could not send message. Try again later...\n");
        }
        strcpy(buffer, msgReceived); /* copys message to be available on main */
        return sent;
}


/*
 *  Function:
 *    REG
 *
 *  Description:
 *    sends register message to snp server. receives feedback from it with sendProtocolMessage function
 *
 *  Arguments:
 *    client characteristics
 *    snp server socket
 *    server connection characteristics
 *
 *
 *  Return value:
 *    1/0 in case of success or failure
 *
 */

int REG(char name[128], char surname[128], char ip[128], char scport[128], int socketfd, struct sockaddr_in serveraddr){
        char *buffer;
        char header[128];
        buffer = calloc(128, sizeof(char));

        sprintf(buffer, "REG %s.%s;%s;%s", name, surname, ip, scport);
        printf("%s\n", buffer);
        if(!sendProtocolMessage(buffer, socketfd, serveraddr)) {

                free(buffer);
                return 0;
        }
        sscanf(buffer, "%s ", header);
        if(strcmp(header, "OK")!=0&&strcmp(header, "NOK")!=0) {
                printf("Could not decipher message received, you should try again...\n");
                free(buffer);
                return 0;
        }
        free(buffer);
        return 1;

}

/*
 *  Function:
 *    UNR
 *
 *  Description:
 *    sends unregister message to snp server. receives feedback from it with sendProtocolMessage function
 *
 *  Arguments:
 *    client characteristics
 *    snp server socket
 *    server connection characteristics
 *
 *
 *  Return value:
 *    1/0 in case of success or failure
 *
 */

int UNR(char name[128], char surname[128], int socketfd, struct sockaddr_in serveraddr){

        char *buffer;
        char header[128];
        buffer = calloc(128, sizeof(char));

        sprintf(buffer, "UNR %s.%s", name, surname);
        printf("%s\n", buffer);
        if(!sendProtocolMessage(buffer, socketfd, serveraddr)) {
                return 0;
                free(buffer);
        }
        sscanf(buffer, "%s ", header);
        if(strcmp(header, "OK")!=0&&strcmp(header, "NOK")!=0) {
                printf("Could not decipher message received, you should try again...\n");
                return 0;
        }
        free(buffer);
        return 1;
}


/*
 *  Function:
 *    QRY
 *
 *  Description:
 *    implements QRY protocol on schat
 *
 *  Arguments:
 *    client characteristics
 *    snp server socket
 *    server connection characteristics
 *
 *
 *  Return value:
 *    1/0 in case of success or failure
 *    saves found client ip and port on contactport and contactip
 *
 */

int QRY(char parametros[128], int socketfd, struct sockaddr_in serveraddr, char *contactip, char *contactport){
        char *buffer, *port, *ip, header[128];
        buffer = calloc(128, sizeof(char));

        sprintf(buffer, "QRY %s", parametros);
        printf("%s\n", buffer);
        sendProtocolMessage(buffer, socketfd, serveraddr);
        sscanf(buffer, "%s ", header);

        if(strcmp(buffer, "RPL")==0) {
                free(buffer);
                return 0;
        } if(strcmp(header, "NOK")==0) {
                free(buffer);
                return 0;
        }
        strtok(buffer, ";");
        ip=strtok(NULL, ";");
        port=strtok(NULL, "\0");
        strcpy(contactip, ip);
        strcpy(contactport, port);
        free(buffer);
        return 1;
}

/*
 *  Function:
 *    read_string
 *
 *  Description:
 *    reads and dynamic allocates memory for a stdin input
 *
 *  Arguments:
 *    none
 *
 *
 *  Return value:
 *    pointer to string read
 *
 */

char * read_string(void){
        char *str = (char*)malloc(sizeof(char));
        int inputFlag=1;
        int i = 0;
        int j = 2;
        char inputByte;
        while (inputFlag) {
                inputByte = getchar();
                if(inputByte=='\n') {
                        inputFlag=0;
                }
                else{
                        str = (char*)realloc(str,j*sizeof(char));
                        str[i] = inputByte;
                        i++;
                        j++;
                }
        }

        str[i]='\0';

        return str;
}

int main(int argc, char *argv[]){


        int i, k=0;
        char *reader, name[128], surname[128], contact_cred[128], contactname[128], contactsurname[128],
              ip[128], scport[128], snpip[128], snpport[128], file[128], fileadd[128], byte[128];

        char *contactip, *contactport;
        contactport = calloc(128, sizeof(char));
        contactip = calloc(128, sizeof(char));

        int parametrosFlag=0;

        int sair=1;

        FILE *fp;

        fd_set rfds;
        int maxfd, counter;
        char buffer[128];
        char *input;
        int len;

        char header[128], *parametros;

        struct sockaddr_in name_server, me_server, contact_server;
        int name_socket, me_socket, contact_socket;
        int contact_flag = 0, me_flag = 0, join_flag=0; /*helping flags to manage the program state. eg, when in connection with another user, contact_flag is 1*/
        unsigned int addrlen;
        unsigned char c;
        unsigned int auth_sent, auth_recv, auth_file, line;

        void (*old_handler)(int);

        if((old_handler=signal(SIGPIPE, SIG_IGN))==SIG_ERR) {
                printf("Error on writing\n");
                exit(1);
        }

        if(argc!=11) { /* Number of arguments inserted is not correct */
                printf("Something went wrong...\nUsage: schat -n <name>.<survame> -i <ip> -p <scport> -s <snpip> -q <snpport>\n");
                exit(4);
        }else{ /*Reading each one of the arguments */
                for(i=1; i<argc; i=i+2) {

                        switch(argv[i][1]) {

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
                                printf("Something went wrong...\nUsage: schat -n <name>.<survame> -i <ip> -p <scport> -s <snpip> -q <snpport>\n");
                                exit(-1);
                                break;
                        }
                }
        }
        printf("Hello %s %s! You are under the IP %s:%s. To register on %s:%s type join.\n", name, surname, snpip, snpport, ip, scport);

        /* creating udp client to connect with snp server */
        name_socket= newudpclient(&name_server, snpip, IP, snpport);

        /* tcp server to receive connect requests from other schats */
        me_socket= newtcpserver(&me_server, scport);

        if(bind(me_socket, (struct sockaddr*)&me_server, sizeof(me_server)) < 0) {
                printf("Error on binding\n");
                exit(1);
        }
        if(listen(me_socket,2)==-1) {
                printf("Error on listening\n");
                exit(1);
        }

        while(sair) {
                /* on each cicle, we SET only the file decriptors needed, eg. if we are joined to a snp server, theres no need to
                set the udp client socket or the tcp server socket even though they exist */
                parametrosFlag = 0;
                FD_ZERO(&rfds);
                FD_SET(STDIN, &rfds);
                maxfd = STDIN;
                if(contact_flag) {
                        FD_SET(contact_socket, &rfds);
                        if(contact_socket>maxfd)
                                maxfd = contact_socket;
                } if(me_flag) {
                        FD_SET(me_socket, &rfds);
                        if(me_socket>maxfd)
                                maxfd = me_socket;
                }

                counter = select(maxfd+1, &rfds, NULL, NULL, NULL);

                if(counter<0) {
                        printf("Error on select\n");
                        exit(4);
                }else if(counter>0) {

                        if(FD_ISSET(STDIN, &rfds)) {

                                input = read_string();  /* read from stdin */
                                if(strlen(input)>0) { /* this is to avoid cases where user clicks on ENTER with nothing written */
                                        sscanf(input, "%s ", header); /* read header from input */

                                        if(strlen(input) != strlen(header)) { /* if there is more than the header to read, like the message command, we need to alloc mpore memory for those parammeters */
                                                parametros =  calloc(strlen(input) - strlen(header), sizeof(char));
                                                sscanf(input, "%s %[^\t\n]", header, parametros);
                                                parametrosFlag = 1;
                                        }



                                        if(strcmp(header, "join")==0) {
                                                if(!join_flag) {
                                                        if(REG(name, surname, ip, scport, name_socket, name_server)) {
                                                                me_flag = 1;
                                                                join_flag = 1;
                                                        }else{
                                                                printf("Could not join the server, maybe the IP given is not correct.\n");
                                                        }
                                                }else
                                                        printf("You're already registered in a server...\n");




                                        }else if(strcmp(header, "leave")==0) {
                                                if(contact_flag) {
                                                        printf("Please disconnect from your chat call first!\n");
                                                }else{
                                                        if(join_flag) {
                                                                if(!UNR(name, surname, name_socket, name_server))
                                                                        printf("Maybe server went offline... You can exit now and try a new server IP.\n");
                                                                me_flag = 0;
                                                                join_flag = 0;

                                                        }else
                                                                printf("You're not registered in a server...\n");
                                                }





                                        }else if(strcmp(header, "find")==0) {
                                                if(!QRY(parametros, name_socket, name_server, contactip, contactport))
                                                        printf("Could not find %s on the network.\n", parametros);
                                                else
                                                        printf("Client registered under %s:%s.\n", contactip, contactport);





                                        }else if(strcmp(header, "connect")==0) {
                                                if(!join_flag)
                                                        printf("Please join a server first.\n");
                                                else{
                                                        if(!contact_flag) {

                                                                k=0;
                                                                if(sscanf(parametros, "%s %s", header, file)==2) {
                                                                        reader = strtok(header, ".");
                                                                        strcpy(contactname, reader);
                                                                        reader = strtok(NULL, "\0");
                                                                        strcpy(contactsurname, reader);

                                                                        if(strcmp(contactname, name)==0 && strcmp(contactsurname, surname)==0) {
                                                                                printf("You can't connect to yourself!\n");
                                                                        }else{
                                                                                if(!QRY(parametros, name_socket, name_server, contactip, contactport))
                                                                                        printf("Could not find %s %s on the network.\n", contactname, contactsurname);
                                                                                else{
                                                                                        printf("Client registered under %s:%s.\n", contactip, contactport);
                                                                                        sprintf(fileadd,"keyfile/%s.txt", file);
                                                                                        fp = fopen (fileadd, "r");
                                                                                        if(fp==NULL) {
                                                                                                printf("Error opening keyfile. Please check your input and try again.\n");
                                                                                        }else{
                                                                                                contact_socket = newtcpclient(&contact_server, contactip, contactport);
                                                                                                contact_flag=1;

                                                                                                if(connect(contact_socket,(struct sockaddr*)&contact_server, sizeof(contact_server)) < 0) {
                                                                                                        printf("Error connecting.\n");
                                                                                                        close(contact_socket);
                                                                                                        contact_flag=0;
                                                                                                }

                                                                                                sprintf(buffer, "NAME %s.%s", name, surname);

                                                                                                if(write(contact_socket, buffer, strlen(buffer))==-1) {
                                                                                                        printf("Error on writing. Maybe user is already on session. Please try again later.\n");
                                                                                                        exit(1);
                                                                                                }else{
                                                                                                        len=read(contact_socket, buffer, 6);
                                                                                                        if(len==-1) {
                                                                                                                printf("Error on reading. Maybe user is already on session. Please try again later.\n");
                                                                                                                exit(1);
                                                                                                        } if(len==0) {
                                                                                                                printf("Other client closed connection.\n");
                                                                                                                contact_flag=0;
                                                                                                        }else{
                                                                                                                buffer[6] = '\0';
                                                                                                        }
                                                                                                        sscanf(buffer, "%s %c", header, &c);
                                                                                                        if(strcmp(header, "AUTH")!=0) {
                                                                                                                printf("Could not decipher message received...\n");
                                                                                                                exit(1);
                                                                                                        }
                                                                                                        line = (unsigned int) c;

                                                                                                        if(line<=0)
                                                                                                                line = line + 256;

                                                                                                        line = line - 1;

                                                                                                        while ((fgets(byte, sizeof(byte), fp) != NULL )&&(k<line)) {
                                                                                                                k++;
                                                                                                        }
                                                                                                        line = atoi(byte);

                                                                                                        c = (unsigned char) line;

                                                                                                        sprintf(buffer, "AUTH %c", c);
                                                                                                        if(write(contact_socket, buffer, 6)==-1) {
                                                                                                                printf("Error on writing.\n");
                                                                                                                exit(1);
                                                                                                        }

                                                                                                        srand(time(NULL));
                                                                                                        auth_sent = rand() % 256;
                                                                                                        c = (unsigned char) auth_sent;
                                                                                                        sprintf(buffer, "AUTH %c", c);
                                                                                                        if(write(contact_socket, buffer, 6)==-1) {
                                                                                                                printf("Error on writing.\n");
                                                                                                                exit(1);
                                                                                                        }

                                                                                                        len=read(contact_socket, buffer, 6);
                                                                                                        if(len==-1) {
                                                                                                                printf("Error on reading.\n");
                                                                                                                exit(1);
                                                                                                        } if(len==0) {
                                                                                                                printf("Other client closed connection.\n");
                                                                                                                contact_flag=0;
                                                                                                        }else{
                                                                                                                buffer[6] = '\0';
                                                                                                        }

                                                                                                        sscanf(buffer, "%s %c", header, &c);
                                                                                                        if(strcmp(header, "AUTH")!=0) {
                                                                                                                printf("Could not decipher message received...\n");
                                                                                                                exit(1);
                                                                                                        }
                                                                                                        auth_recv = (unsigned int)c;

                                                                                                        if(auth_recv<=0)
                                                                                                                auth_recv = auth_recv + 257;

                                                                                                        line = auth_sent - 1;
                                                                                                        fclose(fp);
                                                                                                        fp = fopen(fileadd, "r");
                                                                                                        if(fp==NULL) {
                                                                                                                printf("Error opening keyfile\n");
                                                                                                                exit(1);
                                                                                                        }
                                                                                                        k=0;

                                                                                                        while ((fgets(byte, sizeof(byte), fp) != NULL )&&(k<line))
                                                                                                                k++;

                                                                                                        auth_file = atoi(byte);

                                                                                                        if(auth_file != auth_recv) {
                                                                                                                printf("Could not authenticate %s %s, closing connection...\n",contactname, contactsurname);
                                                                                                                close(contact_socket);
                                                                                                                contact_flag = 0;
                                                                                                        }else{
                                                                                                                printf("Authentification Complete! You can now send messages to each other!\n");
                                                                                                        }
                                                                                                }
                                                                                                fclose(fp);
                                                                                        }
                                                                                }
                                                                        }
                                                                }else{
                                                                        printf("Please insert a keyfile to authenticate your call.\n");
                                                                }
                                                        }else
                                                                printf("You are already on a call session, please disconnect first before connecting to other users.\n");
                                                }
                                        }else if(strcmp(header, "disconnect")==0) {
                                                if(contact_flag) {
                                                        close(contact_socket);
                                                        contact_flag=0;
                                                        me_flag=1;
                                                        printf("OK\n");
                                                }else
                                                        printf("You are not connected...\n");

                                        }else if(strcmp(header, "message")==0) {

                                                if(contact_flag) {
                                                        int nwritten=0, nleft=0;
                                                        char *ptr = parametros;
                                                        nleft = len = strlen(parametros);
                                                        if(len!=0) {
                                                                parametros[len]='\n';
                                                                while(nleft>0) {
                                                                        nwritten=write(contact_socket, ptr, nleft+1);
                                                                        if(nwritten==-1) {
                                                                                printf("Error on writing\n");
                                                                                exit(1);
                                                                        }else{
                                                                                nleft = nleft - nwritten;
                                                                                ptr = &parametros[nwritten];
                                                                        }
                                                                }
                                                        }
                                                }else
                                                        printf("You are not connected... use connect <name>.<surname> <keyfile> first.\n");
                                        }else if(strcmp(header, "exit")==0) {
                                                if(contact_flag) {
                                                        printf("You need to leave the chat and the name server first. Type disconnect then leave and try again.\n");
                                                }else if(join_flag) {
                                                        printf("You need to leave the name server first. Type leave and try again.\n");
                                                }else
                                                        sair=0;
                                        }else if(strcmp(header, "clear")==0) {
                                                printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
                                        }else{
                                                printf("Header (%s) is not valid.\n", header);
                                        }
                                        if (parametrosFlag) {
                                                free(parametros);
                                        }
                                }
                                memset(input, '\0', strlen(input));
                                free(input);
                        } if(me_flag) {
                                if(FD_ISSET(me_socket, &rfds)) {
                                        if(!contact_flag) {
                                                k=0;
                                                addrlen = sizeof(me_server);
                                                if((contact_socket=accept(me_socket,(struct sockaddr *)&me_server,  &addrlen))==-1) {
                                                        printf("Error on accepting\n");
                                                        exit(1);
                                                }
                                                printf("Someone just connected to you... trying to authenticate.\n");

                                                len=read(contact_socket, buffer, sizeof(buffer));
                                                if(len==-1) {
                                                        printf("Error on reading\n");
                                                        exit(1);
                                                } if(len==0) {
                                                        printf("Other client closed connection.\n");
                                                }else{
                                                        buffer[len] = '\0';
                                                }

                                                sscanf(buffer, "%s %s", header, contact_cred);
                                                if(strcmp(header, "NAME")!=0) {
                                                        printf("Could not decipher message received...\n");
                                                        exit(1);
                                                }
                                                reader = strtok(contact_cred, ".");
                                                strcpy(contactname, reader);
                                                reader = strtok(NULL, "\0");
                                                strcpy(contactsurname, reader);
                                                printf("Name: %s\nSurname: %s\n", contactname, contactsurname);

                                                srand(time(NULL));
                                                auth_sent = rand() % 256;
                                                c = (unsigned char)auth_sent;
                                                sprintf(buffer, "AUTH %c", c);
                                                if(write(contact_socket, buffer, 6)==-1) {
                                                        printf("Error on writing\n");
                                                        exit(1);
                                                }

                                                len=read(contact_socket, buffer, 6);
                                                if(len==-1) {
                                                        printf("Error on reading\n");
                                                        exit(1);
                                                } if(len==0) {
                                                        printf("Other client closed connection.\n");
                                                }else{
                                                        buffer[6] = '\0';
                                                }

                                                sscanf(buffer, "%s %c", header, &c);
                                                if(strcmp(header, "AUTH")!=0) {
                                                        printf("Could not decipher message received...\n");
                                                        exit(1);
                                                }

                                                auth_recv = (unsigned int) c;

                                                if(auth_recv<=0)
                                                        auth_recv = auth_recv + 256;

                                                line = auth_sent - 1;


                                                sprintf(fileadd,"keyfile/%s%s.txt", contactname, contactsurname);
                                                fp = fopen (fileadd, "r");

                                                if(fp==NULL) {
                                                        printf("Error opening keyfile\n");
                                                }
                                                else{
                                                        while ((fgets(byte, sizeof(byte), fp) != NULL )&&(k<line))
                                                                k++;

                                                        auth_file = atoi(byte);

                                                        if(auth_file != auth_recv) {
                                                                printf("Could not authenticate %s %s, closing connection...\n",contactname, contactsurname);
                                                                close(contact_socket);
                                                        }else{



                                                                len=read(contact_socket, buffer, 6);
                                                                if(len==-1) {
                                                                        printf("Error on reading\n");
                                                                        exit(1);
                                                                } if(len==0) {
                                                                        printf("Other client closed connection.\n");
                                                                }else{
                                                                        buffer[6] = '\0';
                                                                }
                                                                sscanf(buffer, "%s %c", header, &c);
                                                                if(strcmp(header, "AUTH")!=0) {
                                                                        printf("Could not decipher message received...\n");
                                                                        exit(1);
                                                                }
                                                                line = (unsigned int)c;

                                                                if(line<=0)
                                                                        line = line + 256
                                                                        ;
                                                                line = line -1;

                                                                k=0;
                                                                fclose(fp);
                                                                fp = fopen (fileadd, "r");
                                                                while ((fgets(byte, sizeof(byte), fp) != NULL )&&(k<line)) {
                                                                        k++;
                                                                }
                                                                line = atoi(byte);

                                                                c=(unsigned char)line;

                                                                sprintf(buffer, "AUTH %c", c);
                                                                if(write(contact_socket, buffer, 6)==-1) {
                                                                        printf("Error on writing\n");
                                                                        exit(1);
                                                                }
                                                                fclose(fp);
                                                                contact_flag=1;
                                                        }
                                                }
                                        }else{
                                                int third_connection;
                                                addrlen = sizeof(me_server);
                                                if((third_connection=accept(me_socket,(struct sockaddr *)&me_server,  &addrlen))==-1) {
                                                        printf("Error on accepting\n");
                                                        exit(1);
                                                }
                                                close(third_connection);
                                        }
                                }
                        } if(contact_flag) {
                                if(FD_ISSET(contact_socket, &rfds)) {
                                        char *str = (char*)malloc(sizeof(char));
                                        int msgFlag=1;
                                        int i = 0;
                                        int j = 2;
                                        char msgByte;
                                        int msgClose = 0;
                                        while (msgFlag) {
                                                len=read(contact_socket, &msgByte, sizeof(char));
                                                if(len==-1) {
                                                        printf("Error on reading\n");
                                                        exit(1);
                                                } if(len==0) {
                                                        printf("Other client closed connection.\n");
                                                        close(contact_socket);
                                                        contact_flag=0;
                                                        msgFlag=0;
                                                }else{
                                                        if(msgByte=='\n') {
                                                                msgFlag=0;
                                                                msgClose=1;
                                                        }
                                                        else{
                                                                str = (char*)realloc(str,j*sizeof(char));
                                                                str[i] = msgByte;
                                                                i++;
                                                                j++;
                                                        }
                                                }
                                        }
                                        if(msgClose) {
                                                str[i]='\0';
                                                printf("%s %s: %s\n", contactname, contactsurname, str);
                                        }
                                        free(str);
                                }
                        }
                }
        }
        if(me_flag)
                close(me_socket);
        close(name_socket);

        free(contactport);
        free(contactip);
        return 0;
}
