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
	char sair[128];
	/*struct hostent *h;
	struct in_addr *a;
	*/
	
	socketfd=socket(AF_INET, SOCK_DGRAM, 0);
	if(socketfd==-1){
		printf("Error opening socket");
		exit(1);
	}
	/*
	if((h=gethostbyname("lima"))==NULL){
		printf("Error on getting host");
		exit(1);
	}
	
	printf("official host name: %s\n", h->h_name);
	a=(struct in_addr*)h->h_addr_list[0];*/

	memset((void*)&serveraddr,(int)'\0',sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serveraddr.sin_port=htons((u_short)PORT);
	
	if(bind(socketfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
		printf("Error on binding\n");
		exit(1);
	}
	
	addrlen = sizeof(clientaddr);
	
	printf("se pretender fechar o programa escreva sair\n");
	
	while(strcmp(buffer,"sair")!=0){
		if((recvfrom(socketfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientaddr, &addrlen))==-1){
			printf("Error on recvfrom");
			exit(1);
		}
		
		
		
		printf("%s\n", buffer);
		
		fgets(buffer, sizeof(buffer), stdin);
		sscanf(buffer, "%s\n", buffer);
		
		if(sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&clientaddr, addrlen)==-1){
			printf("Error sending\n");
			exit(1);
		}
	}
	
	
	close(socketfd);
	
	return 0;
}





