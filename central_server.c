#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


struct node
{
	char port[128];
	char ip_addr[128];
	char handle[128];
};

struct node* user_table[1024];
int table_size = 0;


struct node* search(char a[])
{
	printf("%d\n",table_size); 

	for(int i=0;i<table_size;i++) 
	{
		printf(":%s\n",user_table[i]->handle);

		if(strcmp(user_table[i]->handle,a)==0) 
		{
			return user_table[i];
		} 
	} 

	return NULL;
} 

void insert_replace(struct node* x) 
{
	for(int i=0;i<table_size;i++) 
	{
		if(strcmp(user_table[i]->handle,x->handle)==0) 
		{
			user_table[i] = x;
			return;
		} 
	} 
	user_table[table_size] = x;
	table_size++;
	printf("inserting entry %d",table_size);
}

char out_buff[1024];
char in_buff[1024];
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

int check_prefix(char t[],char p[]) 
{
	int i=0;

	while(p[i] != '\0') 
	{
		if(p[i] != t[i]) return 1;
		i++;
	}
	return 0;
} 

void *server_sock(void *socket){

	int *socketfd;
	socketfd=(int*)(socket);
	int newsocket=*socketfd;
	
	memset(in_buff,'\0',sizeof in_buff);
	recv(newsocket,in_buff,1024*sizeof(char),0);
	
	if(strcmp(in_buff,"connect")==0)
	{
		recv(newsocket,in_buff,1024*sizeof(char),0);            //receive handle
		printf("asking for socket!, table size : %d\n",table_size); 
		struct node* temp = search(in_buff);
		printf("asking for socket!\n"); 
	
		if(temp == NULL)
		{
			printf("%s\n",in_buff);
			strcpy(out_buff,"0");								//status code
			send(newsocket,out_buff,strlen(out_buff),0);
			strcpy(out_buff,"USER WITH GIVEN HANDLE NOT CURRENTLY ONLINE");
			send(newsocket,out_buff,strlen(out_buff),0);
			return NULL;
		}

		strcpy(out_buff,"1");									//status code 
		send(newsocket,out_buff,strlen(out_buff),0);
		sleep(0.5);  
		send(newsocket,temp->ip_addr,strlen(temp->ip_addr),0);        //send port number 
		sleep(0.5);  
		send(newsocket,temp->port,strlen(temp->port),0);  //send IP address
		sleep(0.5);  
	}
	else if(strcmp(in_buff,"add")==0)
	{
		struct node *temp = (struct node*)malloc(sizeof(struct node));
		recv(newsocket,temp->handle,sizeof(temp->handle),0);            //receive handle
		printf("handle :%s\n",temp->handle); 
		recv(newsocket,temp->ip_addr,sizeof(temp->ip_addr),0);            //receive port number
		recv(newsocket,temp->port,sizeof(temp->port),0);            //receive IP address
		insert_replace(temp);
		printf("table size after insert: %d\n",table_size); 
	}
	else
	{
		strcpy(out_buff,"ERROR: Request Format not recognizible"); 
		send(newsocket,out_buff,strlen(out_buff),0);
	}
	pthread_exit(0); 	
}

int main(int argc,char* argv[]){
	int socketfd,newsocket;
	struct sockaddr_in serverAddr,cliAddr;
	socklen_t cli_size;
	socketfd = socket(PF_INET,SOCK_STREAM,0);
	bzero((char *)&serverAddr,sizeof serverAddr);
	serverAddr.sin_family = AF_INET;
	if(argc<2) fprintf(stderr,"No port provided.\n");
	serverAddr.sin_port = htons(atoi(argv[1]));
	serverAddr.sin_addr.s_addr= INADDR_ANY;

	if(bind(socketfd,(struct sockaddr*)&serverAddr,sizeof serverAddr)<0){
		fprintf(stderr,"Error binding socket.\n");
		exit(1);
	}
	if(listen(socketfd,5)!=0){
		fprintf(stderr, "Error making socket passive.\n");
		exit(1);
	}
	while(1){
		cli_size = sizeof cliAddr;
		newsocket = accept(socketfd,(struct sockaddr*)&cliAddr,&cli_size);
		//int pid = fork();
		//if(pid==0)
		pthread_t thread_read,thread_write;
		// pthread_create(&thread_read,NULL,readsock,(void*)&newsocket);
		pthread_create(&thread_write,NULL,server_sock,(void*)&newsocket);
		// pthread_detach(thread_read);
		pthread_detach(thread_write);
		
	}
	pthread_exit(0);
}
