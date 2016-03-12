#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define PORT 58000

int main(){

	int socketfd;
	struct hostent *hostptr;
	struct sockaddr_in serveraddr, clientaddr;
	int addrlen;
	char buffer[128];
	char server[50];
	struct hostent *h;
	struct in_addr *a;
	int sair =1;
	
	
	socketfd=socket(AF_INET, SOCK_DGRAM, 0);
	if(socketfd==-1){
		printf("Error opening socket\n");
		exit(1);
	}
	printf("Conectar a: ");
	fgets(server, sizeof(server), stdin);
	sscanf(server, "%s\n", server);
	/*if((h=gethostbyname("porto"))==NULL){
		printf("Error on getting host\n");
		exit(1);
	}
	
	printf("official host name: %s\n", h->h_name);
	a=(struct in_addr*)h->h_addr_list[0];*/

	memset((void*)&serveraddr,(int)'\0',sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	inet_pton(AF_INET, "194.210.224.191", &(serveraddr.sin_addr));	
	serveraddr.sin_port=htons((u_short)PORT);
	
	/*if(bind(socketfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
		printf("Error on binding");
		exit(1);
	}
	*/
	addrlen = sizeof(serveraddr);

	while(sair){
		printf("Mensagem a Enviar para %s: ", server);
		fgets(buffer, sizeof(buffer), stdin);
		if(strcmp(buffer, "sair")==0){
			sair = 0;
			close(socketfd);
		}

		if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&serveraddr, addrlen)==-1){
			printf("Error sending\n");
			exit(1);
		}
	}
	return 0;
}





