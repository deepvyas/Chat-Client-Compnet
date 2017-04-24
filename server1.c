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
	
	int socket_cs;
	int socketfd,newsocket;
	struct sockaddr_in serverAddr,cliAddr,cServerAddr;
	char interface[] = "wlp3s0";
	char cs_addr[128];

	socklen_t cli_size;
	socketfd = socket(PF_INET,SOCK_STREAM,0);
	socket_cs = socket(PF_INET,SOCK_STREAM,0);
	bzero((char *)&serverAddr,sizeof serverAddr);
	bzero((char *)&cServerAddr,sizeof cServerAddr);
	cServerAddr.sin_family = AF_INET;
	serverAddr.sin_family = AF_INET;
	if(argc<2) fprintf(stderr,"No port provided.\n");
	serverAddr.sin_port = htons(atoi(argv[1]));
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	cServerAddr.sin_port = htons(8000);
	cServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");	//write server add here


	if(connect(socket_cs,(struct sockaddr*)&cServerAddr,sizeof cServerAddr )<0)
	{
		fprintf(stderr,"CANNOT CONNECT TO CENTRAL CHAT SERVER");
	}

	printf("Enter your chat system handle\n");
	scanf("%s",handle);

	printf("Enter your chat server ip address\n"); 
	scanf("%s",cs_addr);

	strcpy(out_buff,"add");
	send(socket_cs,out_buff,strlen(out_buff),0);
	send(socket_cs,handle,strlen(handle),0);
	send(socket_cs,cs_addr,strlen(cs_addr),0);
	send(socket_cs,argv[1],strlen(argv[1]),0);

	close(socket_cs); 

	if(bind(socketfd,(struct sockaddr*)&serverAddr,sizeof serverAddr)<0){
		fprintf(stderr,"Error binding socket.\n");
		exit(1);
	}
	if(listen(socketfd,1)!=0){
		fprintf(stderr, "Error making socket passive.\n");
		exit(1);
	}

	while(1){
		cli_size = sizeof cliAddr;
		newsocket = accept(socketfd,(struct sockaddr*)&cliAddr,&cli_size);
		int pid = fork();
		if(pid==0){
			pthread_t thread_read,thread_write;
			pthread_create(&thread_read,NULL,readsock,(void*)&newsocket);
			pthread_create(&thread_write,NULL,writesock,(void*)&newsocket);
			pthread_detach(thread_read);
			pthread_detach(thread_write);
			break;
		}
		else{
			close(newsocket);
		}
	}

	pthread_exit(0);
}
