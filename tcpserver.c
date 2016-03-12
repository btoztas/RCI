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

	int socketfd, newsocketfd;
	struct hostent *hostptr;
	struct sockaddr_in serveraddr, clientaddr;
	int addrlen, clientlen;
	char buffer[128];
	/*struct hostent *h;
	struct in_addr *a;
	*/
	
	socketfd=socket(AF_INET, SOCK_STREAM, 0);
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
		printf("Error on binding");
		exit(1);
	}
	
	if(listen(socketfd, 5)==-1) {
		printf("Error on listening");
		exit(1);
	}

	clientlen = sizeof(clientaddr);
	if(newsocketfd=accept(socketfd, (struct sockaddr*)&clientaddr, &clientlen)){
		printf("Error on accepting");
		exit(1);
	}

	if(read(newsocketfd, buffer, sizeof(buffer))){
		printf("Error on binding");
		exit(1);
	}
	printf("%s\n", buffer);
	
	if(write(newsocketfd, buffer, sizeof(buffer))){
		printf("Error on binding");
		exit(1);
	}
	
	close(newsocketfd);

	close(socketfd);
	
	return 0;
}





