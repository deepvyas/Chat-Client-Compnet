#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
char out_buff[1024];
char in_buff[1024];
int slave;
int master;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
char handle[128];

int socket_cs;
int socketfd,newsocket;
struct sockaddr_in serverAddr,cliAddr,cServerAddr;


void* readsock(void *socket){
	int *socketfd;
	socketfd=(int*)(socket);
	int newsocket=*socketfd;
	while(1){
		recv(newsocket,in_buff,1024*sizeof(char),0);
		// printf("%s\n",in_buff);
		printf("%s\n",in_buff);
		// fflush(slave);
		memset(in_buff,0,1024*sizeof(char));
	}
}

void *writesock(void *socket){
	int *socketfd;
	socketfd=(int*)(socket);
	int newsocket=*socketfd;
	while(1){
		fgets(out_buff,1024,stdin);
		// printf("%s\n",out_buff);
		send(newsocket,out_buff,strlen(out_buff),0);
		sleep(0.2);
		memset(out_buff,0,1024*sizeof(char));
	}
}

void intr_handler(){
	if(connect(socket_cs,(struct sockaddr*)&cServerAddr,sizeof cServerAddr )<0)
	{
		fprintf(stderr,"CANNOT CONNECT TO CENTRAL CHAT SERVER");
	}
	strcpy(out_buff,"sub");
	send(socket_cs,handle,strlen(handle),0);
	close(socket_cs);
	pthread_exit(0);
}

int main(int argc,char* argv[]){
	
	
	char interface[] = "wlp3s0";
	char cs_addr[128];

	socklen_t cli_size;
	socketfd = socket(PF_INET,SOCK_STREAM,0);
	memset((char *)&serverAddr,0,sizeof serverAddr);
	socket_cs = socket(PF_INET,SOCK_STREAM,0);
	memset((char *)&cServerAddr,0,sizeof cServerAddr);
	cServerAddr.sin_family = AF_INET;
	serverAddr.sin_family = AF_INET;
	if(argc<2) fprintf(stderr,"No port provided.\n");
	serverAddr.sin_port = htons(atoi(argv[1]));
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	cServerAddr.sin_port = htons(8000);
	cServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");	//write server add here
	if(bind(socketfd,(struct sockaddr*)&serverAddr,sizeof serverAddr)<0){
		fprintf(stderr,"Error binding socket.\n");
		exit(1);
	}

	if(connect(socket_cs,(struct sockaddr*)&cServerAddr,sizeof cServerAddr )<0)
	{
		fprintf(stderr,"CANNOT CONNECT TO CENTRAL CHAT SERVER TO SEND CLOSURE..");
		pthread_exit(0);
	}

	printf("Enter your chat system handle\n");
	scanf("%s",handle);

	printf("Enter your chat server ip address\n"); 
	scanf("%s",cs_addr);

	strcpy(out_buff,"add");
	send(socket_cs,out_buff,strlen(out_buff),0);
	sleep(0.5); 
	send(socket_cs,handle,strlen(handle),0);
	sleep(0.5); 
	send(socket_cs,cs_addr,strlen(cs_addr),0);
	sleep(0.5); 
	send(socket_cs,argv[1],strlen(argv[1]),0);

	close(socket_cs); 

	if(listen(socketfd,5)!=0){
		fprintf(stderr, "Error making socket passive.\n");
		exit(1);
	}

	while(1){
		signal(SIGINT,intr_handler);
		cli_size = sizeof cliAddr;
		newsocket = accept(socketfd,(struct sockaddr*)&cliAddr,&cli_size);
		int pid = fork();
		if(pid==0){
			printf("\nConnection Request from : %s:%d\n Accept[y/n]?",inet_ntoa(cliAddr.sin_addr),ntohs(cliAddr.sin_port));
			char opt;
			scanf(" %c",&opt);
			if(opt!='y'&&opt!='n'){
				printf("Incorrect option!!\nAccept[y/n]?");
				while(opt!='y'&&opt!='n'){
					scanf("%c",&opt);
					printf("Incorrect option!!\nAccept[y/n]?");
				}
			}
			if(opt=='n'){
				char rej[5]="r";
				send(newsocket,rej,strlen(rej),0);
				break;
			}
			else if(opt=='y'){
				master = posix_openpt(O_RDWR);
				grantpt(master);
				unlockpt(master);
				char command[1000];
				sprintf(command,"xterm -S%s/%d",strrchr(ptsname(master),'/')+1,master);
				/*if(!fork()) {
	   			execlp("xterm", "xterm", command, (char *)0);
	    		_exit(1);
	  		}*/
	  		slave=open(ptsname(master), O_RDWR | O_NOCTTY);
	  		FILE *xtermstdout = popen(command, "r");
	  		dup2(slave, STDIN_FILENO);
	  		dup2(slave, STDOUT_FILENO);
	  		// printf("This appears in the terminal window.\n");
	  		fgets(command, sizeof command,stdin);
	  		// printf("window: %s\n", command);
				pthread_t thread_read,thread_write;
				pthread_create(&thread_write,NULL,writesock,(void*)&newsocket);
				pthread_create(&thread_read,NULL,readsock,(void*)&newsocket);
				pthread_detach(thread_write);
				/*while(recv(newsocket,in_buff,1024*sizeof(char),0)){
					// printf("%s\n",in_buff);
					in_buff[strlen(in_buff)+1]='\0';
					in_buff[strlen(in_buff)+1]='\n';
					puts(in_buff);
					// fflush(slave);
					// printf("PUTs to slave\n");
					memset(in_buff,0,1024*sizeof(char));
				}*/
				pthread_detach(thread_read);
				break;
			}
		}
		else{
			close(newsocket);
		}
	}
	pthread_exit(0);
}
