#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
char out_buff[1024];
char in_buff[1024];

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
	
	while(listen(socketfd,5)==0){
		cli_size = sizeof cliAddr;
		newsocket = accept(socketfd,(struct sockaddr*)&cliAddr,&cli_size);
		//Login Logic
		int bytes_sent,bytes_received;
		sprintf(out_buff,"U");
		bytes_sent = write(newsocket,&out_buff,sizeof out_buff);
		bytes_received = read(newsocket,&in_buff,sizeof in_buff);
		char user[100];
		int flg=-1;
		for(int i=0;i<10;i++){
			if(strcmp(in_buff,users[i])==0){
				strcpy(user,in_buff);
				flg=i;
				break;
			}
		}
		if(flg!=-1){
			sprintf(out_buff,"P");
			bytes_sent = write(newsocket,&out_buff,sizeof out_buff);
			bytes_received = read(newsocket,&in_buff,sizeof in_buff);
			if(strcmp(in_buff,pass[flg])==0){
				sprintf(out_buff,"L");
				bytes_sent = write(newsocket,&out_buff,sizeof out_buff);
			}
			else{
				sprintf(out_buff,"X");
				bytes_sent = write(newsocket,&out_buff,sizeof out_buff);
			}
		}
		else{
			sprintf(out_buff,"X");
			bytes_sent = write(newsocket,&out_buff,sizeof out_buff);
		}
	}
	return 0;
}
