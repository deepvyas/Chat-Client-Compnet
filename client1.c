#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

int u_sock[256];
int nou=0;
char out_buff[1024];
char in_buff[1024];
char handle[128];
char handle_g[128];
char user_ip[128];
char user_port[128];

void* readsock(void *socket){
	int *socketfd;
	socketfd=(int*)(socket);
	int newsocket=*socketfd;
	while(recv(newsocket,in_buff,1024*sizeof(char),0)){
		if(strcmp(in_buff,"r")==0){
			printf("Connection Rejected from user.!! Try again later\n");
			exit(0);
		}
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

void* readgsock(void *socket){
	int *socketfd;
	socketfd=(int*)(socket);
	int newsocket=*socketfd;

	while(recv(newsocket,in_buff,1024*sizeof(char),0)){

		for(int i=0;i<nou;i++) 
		{
			if(u_sock[i] != newsocket) 
				send(u_sock[i],in_buff,1024*sizeof(char),0); 
		}
		printf("%s",in_buff); 
	}
}

void *writegsock(void *socket){
	int *socketfd;
	socketfd=(int*)(socket);
	int newsocket=*socketfd;
	while(fgets(out_buff,1024,stdin)){

		for(int i=1;i<nou;i++) 
		{
				send(u_sock[i],in_buff,1024*sizeof(char),0); 
		}

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

	//if(argc<2) fprintf(stderr,"No port provided.\n");
	serverAddr.sin_port = htons(8000);
	serverAddr.sin_addr.s_addr= inet_addr("127.0.0.1"); 
	if(connect(socketfd,(struct sockaddr*)&serverAddr,sizeof serverAddr)<0){
		fprintf(stderr,"CANNOT CONNECT TO CENTRAL CHAT SERVER\n");
		exit(1);
	}
	int uorg;
	memset(handle,'\0',sizeof handle);
	printf("---Welcome to CHAT CLIENT---\n\n");
	printf("Enter 1 if you want to connect to a user.\n");
	printf("Enter 2 if you want to connect to a group.\n");
	printf("Enter 3 if you want to create a group.\n");
	scanf("%d",&uorg);
	if(uorg==1){
		printf("Enter the handle to user who want to connect with :\n");
		scanf("%s",handle);
		strcpy(out_buff,"connect");
		send(socketfd,out_buff,strlen(out_buff),0);
		sleep(0.5); 
		send(socketfd,handle,strlen(handle),0);
		sleep(0.5);	
		recv(socketfd,in_buff,1024*sizeof(char),0);

		printf("status code recvd %s\n",in_buff);

		if(strcmp(in_buff,"0")==0)
		{
			recv(socketfd,in_buff,1024*sizeof(char),0);
			printf("%s\n",in_buff); 
			exit(1);
		}
		else
		{
			recv(socketfd,user_ip,128*sizeof(char),0);
			sleep(0.5); 
			recv(socketfd,user_port,128*sizeof(char),0);

			printf("Socket recvd\n");
			printf("%s %s\n",user_ip,user_port);

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
	else if(uorg==2){
		printf("Enter the name of the group you want to connect to:\n");
		scanf("%s",handle_g);
		strcpy(out_buff,"connect_g");
		send(socketfd,out_buff,strlen(out_buff),0);
		sleep(0.5); 
		send(socketfd,handle_g,strlen(handle_g),0);
		//Central Server sends back pass(1)or fail(0) status flags  "
		recv(socketfd,in_buff,1024*sizeof(char),0);
		if(strcmp(in_buff,"1")==0){
			printf("Successfully added to group\n");
		}
		else{
			printf("Error: Not added to group\n");
		}
		recv(socketfd,in_buff,1024*sizeof(char),0);
		printf("status code recvd %s\n",in_buff);
		if(strcmp(in_buff,"0")==0)
		{
			recv(socketfd,in_buff,1024*sizeof(char),0);
			printf("%s\n",in_buff); 
		}
		else
		{
			printf("Leader Socket recvd\n");
			recv(socketfd,user_ip,128*sizeof(char),0);
			sleep(0.5); 
			recv(socketfd,user_port,128*sizeof(char),0);

			printf("LeaderSocket recvd\n");
			printf("%s %s\n",user_ip,user_port);

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
	else if(uorg==3){
		printf("Enter Handle of group you want to create.\n");
		scanf("%s",handle_g);
		strcpy(out_buff,"g_create");
		send(socketfd,out_buff,strlen(out_buff),0);
		printf("before : %s",in_buff);
		//sleep(0.5);
		memset(in_buff,'\0',sizeof in_buff); 

		recv(socketfd,in_buff,1024*sizeof(char),0);
		printf("Here i am wwith status : %s\n",in_buff);

		if(strcmp(in_buff,"1")==0){
			printf("%s\n",handle_g); 
			send(socketfd,handle_g,strlen(handle_g),0);
			sleep(0.5);

			recv(socketfd,in_buff,1024*sizeof(char),0);
			
			if(strcmp(in_buff,"0")==0){
				printf("Group Name Taken!!\n");
				exit(1);
			}

			printf("hello--%s",in_buff); 
			
			memset(out_buff,'\0',sizeof out_buff);
			char cs_addr[100];
			printf("Enter your Group server ip address\n"); 
			scanf("%s",cs_addr);
			//strcpy(out_buff,cs_addr);
			send(socketfd,cs_addr,sizeof cs_addr,0);
			sleep(0.5);

			memset(out_buff,'\0',sizeof out_buff);

			char  cs_port[101];
			printf("Enter your Group server port\n");
			scanf("%s",cs_port); 
			send(socketfd,cs_port,strlen(cs_port),0);
			sleep(0.5);

			struct sockaddr_in gserver,user_addr;
			socklen_t g_size;
			int gsocket;

			gsocket = socket(PF_INET,SOCK_STREAM,0);
			gserver.sin_family = AF_INET;
			gserver.sin_port = htons(atoi(cs_port));
			gserver.sin_addr.s_addr = INADDR_ANY;

			if(bind(gsocket,(struct sockaddr*)&gserver,sizeof gserver))
			{
				fprintf(stderr,"Error binding socket.\n");
				exit(1);
			}

			if(listen(gsocket,5)!=0) 
			{
				fprintf(stderr,"Error making socket passive.\n");
				exit(1); 
			}

			while(1)
			{
				g_size = sizeof user_addr;
				u_sock[nou] = accept(gsocket, (struct sockaddr*)&user_addr, &g_size);

				pthread_t t1,t2;
				pthread_create(&t1, NULL, readgsock, (void*)&u_sock[nou]);
				pthread_create(&t2, NULL, writegsock, (void*)&u_sock[nou]);
				pthread_detach(t1);
				pthread_detach(t2); 

				nou++;
			} 
			
		}
		else{
			printf("Some Error on Server Side!!!.\n");
		}
	}
}
