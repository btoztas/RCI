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
	struct sockaddr_in serveraddr;
	int clientlen;
	char buffer[128], msgRecieved[128];
	int n;
	
	
	
	memset((void*)&(buffer),(int)'\0',sizeof((buffer)));
	
	socketfd=socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd==-1){
	  printf("Error opening socket\n");
	  exit(1);
	}

	memset((void*)&(serveraddr),(int)'\0',sizeof((serveraddr)));
	(serveraddr).sin_family=AF_INET;
	(serveraddr).sin_addr.s_addr=htonl(INADDR_ANY);
	(serveraddr).sin_port=htons((u_short)PORT);

	
	if(bind(socketfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
		printf("Error on binding");
		exit(1);
	}
	
	printf("socket= %d\n", socketfd);
	if(listen(socketfd, 5)==-1) {
		  printf("Error on listening");
		  exit(1);
	}
	
	while(1){
	  
	  clientlen = sizeof(serveraddr);
	  
	  if((newsocketfd=accept(socketfd,(struct sockaddr *)&serveraddr,&clientlen))==-1){
		  printf("Error on accepting");
		  exit(1);
	  }
	  
	  printf("newsocket= %d\n", newsocketfd);
	  
	  memset((void*)&(msgRecieved),(int)'\0',sizeof((msgRecieved)));
	  
	  if(n=read(newsocketfd, msgRecieved, sizeof(msgRecieved))==-1){
		  printf("Error on reading");
		  exit(1);
	  }
	  
	  printf("nread= %d\n", n);
	  
	  printf("%s\n", msgRecieved);
	  
	  sprintf(buffer, "Enviaste-me: %s\n", msgRecieved);
	  
	  if(write(newsocketfd, buffer, sizeof(buffer))==-1){
		  printf("Error on writing");
		  exit(1);
	  }
	}
	
	close(newsocketfd);

	close(socketfd);
	
	return 0;
}