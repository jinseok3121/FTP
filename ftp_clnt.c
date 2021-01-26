#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <assert.h>

#include "readnwrite.h"


#define BUF_SIZE 1024
#define NAME_SIZE 20

void error_handling(char*msg);
void* send_msg(void* arg);
void* recv_msg(void* arg);


int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread;
	void * thread_return;
	
	if(argc!=3)
	{
		printf("Usage : %s <IP> <port> \n", argv[0]);
		exit(1);
	} 
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");
	
	
	printf("----------------------------------Welcome FTP----------------------------------\n");
	printf("Write commend (signup or login)\n\n\n");
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);	
	close(sock);

	return 0;
}

void* send_msg(void* arg)
{
	int sock = *((int*)arg);
	char msg[BUF_SIZE];
	
	while(1)
	{
		printf("commend : ");
		fgets(msg, BUF_SIZE, stdin);
		if(!strcmp(msg, "Q\n") || !strcmp(msg, "q\n"))
		{
			close(sock);
			exit(0);
		}
		
		else if(!strcmp(msg, "SIGNUP\n") || !strcmp(msg, "signup\n"))
		{
			sprintf(msg, "%s", msg);
			write(sock, msg, strlen(msg));
      
			printf("Input your ID : ");
			fgets(msg, 30, stdin);
			write(sock, msg, strlen(msg));
			
			printf("Input your PW : ");
			fgets(msg, 30, stdin);
			write(sock, msg, strlen(msg));
		
			continue;
		}

		else if(!strcmp(msg, "LOGIN\n") || !strcmp(msg, "login\n"))
		{
			sprintf(msg, "%s", msg);
			write(sock, msg, strlen(msg));
      
			printf("Input your ID : ");
			fgets(msg, 20, stdin);
			write(sock, msg, strlen(msg));
			
			printf("Input your PW : ");
			fgets(msg, 20, stdin);
			write(sock, msg, strlen(msg));
			
			sleep(1);
			read(sock, msg, sizeof(msg));
			printf("%s\n", msg);
			
			memset(msg, 0, BUF_SIZE);
			
			continue;	
		}
		
		else if(!strcmp(msg, "LIST\n") || !strcmp(msg, "list\n"))
		{
			sprintf(msg, "%s", msg);
			write(sock, msg, strlen(msg));
			
			memset(msg, 0, BUF_SIZE);
			read(sock, msg, sizeof(msg));
			printf("%s\n", msg);
			memset(msg, 0, BUF_SIZE);
			continue;
		}
		
		else if(!strcmp(msg, "DOWN\n") || !strcmp(msg, "down\n"))
		{
			char filename[20];
			int fd;
			int datalen;
			int str_len;
			sprintf(msg, "%s", msg);
			write(sock, msg, strlen(msg));
			
			printf("Input downlowad file name : ");
			fgets(msg, 20, stdin);
			msg[strnlen(msg, sizeof(msg))-1] = '\0';
			printf("Input save file name: ");
			fgets(filename, 20, stdin);
			filename[strnlen(filename, sizeof(filename))-1] = '\0';
			write(sock, msg, strlen(msg));
			sleep(1);
			
			
			fd = open(filename, O_CREAT | O_WRONLY, S_IRWXU);
			if(fd != -1)
				printf("File open success\n");
			
			read(sock, &datalen, sizeof(int));
			datalen = ntohl(datalen);
			
				
			while((datalen -(str_len = read(sock, msg, sizeof(msg))) >0))
			{
				write(fd, msg, str_len);
				if(str_len < BUF_SIZE)
				{
					break;
				}
			}
			close(fd);
			
			sleep(1);
			memset(msg, 0 ,BUF_SIZE);
			printf("Download Success!!!!\n");
			continue;
		
		}
		
		else if(!strcmp(msg, "UP\n") || !strcmp(msg, "up\n"))
		{
			int fd;
			int file_len;
			int read_len;
			char filename[20];
			
			sprintf(msg, "%s", msg);
			write(sock, msg, strlen(msg));
		
			printf("Input upload file name : ");
			fgets(msg, 20, stdin);
			msg[strnlen(msg, sizeof(msg))-1] = '\0';
			printf("Input save file name : ");
			fgets(filename, 20, stdin);
			write(sock, filename, strlen(filename));
			
			sleep(1);
			
			filename[strnlen(filename, sizeof(filename))-1] = '\0';
			
			fd = open(msg, O_RDONLY, S_IRWXU);
			
			file_len = lseek(fd, 0, SEEK_END);
			file_len = htonl(file_len);
			write(sock, &file_len, sizeof(file_len));
	
			lseek(fd, 0, SEEK_SET);
				
			while((read_len = read(fd, msg, sizeof(msg))) > 0)
			{
				//printf("reading file\n");
				if(write(sock, msg, read_len) !=  read_len)
				{
					error_handling("write() error");
				}
			}
			
		
			sleep(1);
			memset(msg, 0, BUF_SIZE);
			
			printf("Upload Success!!!!\n");
			continue;
		}
		
	
		
	}
	return NULL;	
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
