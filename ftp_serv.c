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
#define MAX_CLNT 256

void * hanlde_clnt(void * arg);
void error_handling(char*msg);

int clnt_cnt =0;
int clnt_socks[MAX_CLNT];
char ID[MAX_CLNT][30] = {0x00, };  
char PW[MAX_CLNT][30] ={0x00, };
int cnt = 0;
pthread_mutex_t mutex;

int main(int argc, char * argv[])
{
	int cnt_i;
	int serv_sock;
	int clnt_sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;
	pthread_t t_id;
	
	if(argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	pthread_mutex_init(&mutex, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
	{
		error_handling("bind() error");
	}
	
	if(listen(serv_sock, 5) == -1)
	{
		error_handling("listen() error");
	}

	
	while(1)
	{
		clnt_addr_size = sizeof(clnt_addr_size);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		
		pthread_mutex_lock(&mutex);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mutex);
	
		pthread_create(&t_id, NULL, hanlde_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("\n[TCP Server] Client connected: IP=%s, port=%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
		
	}
	
	close(serv_sock);
	return 0;
}


void * hanlde_clnt(void *arg)
{
	int clnt_sock = *((int*)arg);
	int str_len = 0, i;   
	char msg[BUF_SIZE];
	
	
	while((str_len = read(clnt_sock, msg, sizeof(msg))) != 0) 
	{
		if(!strcmp(msg, "SIGNUP\n") || !strcmp(msg, "signup\n"))
		{
			
			str_len = read(clnt_sock, msg, sizeof(msg));
			msg[str_len] = '\0';
			pthread_mutex_lock(&mutex);
			memcpy(&ID[cnt], msg, strlen(msg));
			pthread_mutex_unlock(&mutex); 
			printf("NEW ID : %s\n", ID[cnt]);
			
			str_len = read(clnt_sock, msg, sizeof(msg));
			msg[str_len] = '\0';
			pthread_mutex_lock(&mutex);
			memcpy(&PW[cnt], msg, strlen(msg));
			pthread_mutex_unlock(&mutex);
			printf("NEW PW : %s\n", PW[cnt]);
			
			cnt++;
			memset(msg, 0 ,BUF_SIZE);
			
			continue;
		}
		
		else if(!strcmp(msg, "LOGIN\n") || !strcmp(msg, "login\n"))
		{
			char CKID[30];
			char CKPW[30];
			int checkID = 1;
			int checkPW = 1;
			int str_len;
			char err[] = {"Try again login\n"};
			printf("Client login.....\n");
			
			str_len = read(clnt_sock, CKID, sizeof(CKID));
			CKID[str_len] = '\0';
			printf("ID : %s\n", CKID);
			str_len = read(clnt_sock, CKPW, sizeof(CKPW));
			CKPW[str_len] = '\0';
			printf("PW : %s\n", CKPW);
			
			for(int i = 0; i<MAX_CLNT; i++)
			{
				checkID = strcmp(ID[i] , CKID);
				checkPW = strcmp(PW[i] , CKPW);
				
				//printf("ID : %d\n", checkID);
				//printf("PW : %d\n", checkPW);

				if(checkID == 0  && checkPW == 0)
				{
					char com[] = {"write commend(list or down or up)\n"};
					write(clnt_sock, com, sizeof(com));
					memset(msg, 0 ,BUF_SIZE);
					break;
				}
				else
				{
					continue;
					memset(msg, 0 ,BUF_SIZE);
				}
			}
			write(clnt_sock, err, sizeof(err));	
			memset(msg, 0 ,BUF_SIZE);
			continue;
		}
		
		else if(!strcmp(msg, "LIST\n") || !strcmp(msg, "list\n"))
		{
			printf("list.....\n");
		
			int fd;
			int read_len = 0;
			fd = open("directory.txt", O_RDONLY, S_IRWXU);
			
			while((read_len = read(fd, msg, sizeof(msg))) >0)
			{
				write(clnt_sock, msg, read_len);	
			}
			memset(msg, 0 ,BUF_SIZE);
			continue;
		}
		
		else if(!strcmp(msg, "DOWN\n") || !strcmp(msg, "down\n"))
		{
			printf("Download.....\n");
			str_len = read(clnt_sock, msg, sizeof(msg));
			
			int fd;
			int read_len = 0;
			int file_len;
			fd = open(msg, O_RDONLY, S_IRWXU);
			
			file_len = lseek(fd, 0, SEEK_END);
			file_len = htonl(file_len);
			write(clnt_sock, &file_len, sizeof(file_len));
	
			lseek(fd, 0, SEEK_SET);
				
			while((read_len = read(fd, msg, sizeof(msg))) > 0)
			{
				//printf("reading file\n");
				if(write(clnt_sock, msg, read_len) !=  read_len)
				{
					error_handling("write() error");
				}
			}
			
			sleep(1);

			memset(msg, 0 ,BUF_SIZE);	
			
			
		}
		
		else if(!strcmp(msg, "UP\n") || !strcmp(msg, "up\n"))
		{
			int fd;
			int fd2;
			char *filename;
			int datalen;
			
			printf("Upload.....\n");
			str_len = read(clnt_sock, msg, sizeof(msg));
			msg[str_len] = '\0';
			printf("file name :%s", msg);
			filename = msg;
			
			fd2 = open("directory.txt", O_APPEND | O_WRONLY, S_IRWXU);
			write(fd2, filename, strlen(filename));
			close(fd2);
			sleep(1);

			fd = open(filename, O_CREAT | O_WRONLY, S_IRWXU);
			if(fd != -1)
				printf("File open success\n");
				
			read(clnt_sock, &datalen, sizeof(int));
			datalen = ntohl(datalen);
			printf("datalen : %d\n", datalen);
			
			while((datalen -(str_len = read(clnt_sock, msg, sizeof(msg))) >0))
			{
				write(fd, msg, str_len);
				if(str_len < BUF_SIZE)
				{
					break;
				}
			}
			lseek(fd, 0, SEEK_SET);
			close(fd);
			
			memset(msg, 0 ,BUF_SIZE);
			continue;
		
		}			
	}
	
	pthread_mutex_lock(&mutex);
	for(i = 0; i<clnt_cnt; i++)
	{
		if(clnt_sock == clnt_socks[i])
		{
			while(i++<clnt_cnt - 1)
				clnt_socks[i] = clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutex);
	close(clnt_sock);
	
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
