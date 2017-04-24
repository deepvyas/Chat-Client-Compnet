#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
char out_buff[1024];
char in_buff[1024];
char handle[128];
char user_ip[128];
char user_port[128];

void* readsock(void *socket){
	int *socketfd;
	socketfd=(int*)(socket);
	int newsocket=*socketfd;
	while(recv(newsocket,in_buff,1024*sizeof(char),0)){
		printf("%s\n",in_buff);
		fflush(stdout);
		memset(in_buff,0,1024*sizeof(char));
	}
}

void *writesock(void *socket){
	int *socketfd;
	socketfd=(int*)(socket);
	int newsocket=*socketfd;
	while(fgets(out_buff,1024,stdin)){
		send(newsocket,out_buff,strlen(out_buff),0);
		memset(out_buff,0,1024*sizeof(char));
	}
}

int main(int argc,char* argv[]){
	
	int socketfd,socket_p2p;
	struct sockaddr_in serverAddr,userAddr;

	socketfd = socket(PF_INET,SOCK_STREAM,0);
	socket_p2p = socket(PF_INET,SOCK_STREAM,0);

	bzero((char *)&serverAddr,sizeof serverAddr);
	bzero((char *)&userAddr,sizeof userAddr);

	serverAddr.sin_family = AF_INET;
	userAddr.sin_family = AF_INET;

	if(argc<3) fprintf(stderr,"No port provided.\n");

	serverAddr.sin_port = htons(8000);
	serverAddr.sin_addr.s_addr= inet_addr("127.0.0.1");

	if(connect(socketfd,(struct sockaddr*)&serverAddr,sizeof serverAddr)<0){
		fprintf(stderr,"CANNOT CONNECT TO CENTRAL CHAT SERVER\n");
		exit(1);
	}

	printf("---Welcome to CHAT CLIENT---\n\nEnter the handle to user who want to connect with : "); 
	scanf("%s",handle);

	strcpy(out_buff,"connect");
	send(socketfd,out_buff,strlen(out_buff),0);
	send(socketfd,handle,strlen(handle),0);
	recv(socketfd,in_buff,1024*sizeof(char),0);

	if(strcmp(in_buff,"0")==0)
	{
		recv(socketfd,in_buff,1024*sizeof(char),0);
		printf("%s\n",in_buff); 
	}
	else
	{

		recv(socketfd,user_ip,128*sizeof(char),0);
		recv(socketfd,user_port,128*sizeof(char),0);

		userAddr.sin_addr.s_addr = inet_addr(user_ip); 
		userAddr.sin_port = htons(atoi(user_port)); 	
		// create_connection

		if(connect(socket_p2p,(struct sockaddr*)&userAddr,sizeof userAddr)<0){
			
			fprintf(stderr,"CANNOT CONNECT TO REQUESTED CHAT CLIENT\n");
			exit(1);
	}	

			pthread_t thread_read,thread_write;
			pthread_create(&thread_read,NULL,readsock,(void*)&socket_p2p);
			pthread_create(&thread_write,NULL,writesock,(void*)&socket_p2p);
			pthread_detach(thread_read);
			pthread_detach(thread_write);

		pthread_exit(0);

	}
}
