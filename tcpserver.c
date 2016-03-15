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


int main(){

	int socketfd, newsocketfd;
	struct hostent *hostptr;
	struct sockaddr_in serveraddr, clientaddr;
	int addrlen, clientlen;
	char buffer[128];
	/*struct hostent *h;
	struct in_addr *a;
	*/
	
	socketfd=newtcpserver(&serveraddr);
	
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
	sprintf(buffer, "Enviaste-me: %s\n", buffer);
	if(write(newsocketfd, buffer, sizeof(buffer))){
		printf("Error on binding");
		exit(1);
	}
	
	close(newsocketfd);

	close(socketfd);
	
	return 0;
}





