#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#define SERVPORT 3333 /*listen port*/
#define BACKLOG 10 /*maxinum number of connection*/
#define MAXDATASIZE 200 
typedef struct information1{
        int board_id;
        char board_name[40];
        char board_version[40];
        char macaddr[40];
        char time[50];
        char event[50];
}info1;


typedef struct information{
        char addr[19];
	char time[50];
	char mac[32];
}info;
main()
{

        int sock_fd, client_fd,recvbytes; /*sock_fd: listen socket; client_fd: date socket */
        struct sockaddr_in my_addr; /*local machine address*/
        struct sockaddr_in remote_addr; /*client address*/
	info buf;
        if((sock_fd = socket(AF_INET, SOCK_STREAM,0)) == -1)
        {
	  printf("socket failed");
          exit(1);

        }
	printf("*******************\n");
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(SERVPORT);
        my_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(my_addr.sin_zero),8);
        if(bind(sock_fd,(struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
        {
          exit(1);
        }
	printf("*******************\n");
        if(listen(sock_fd,BACKLOG) == -1)
        {
          exit(1);
        }
	printf("*******************\n");
        while(1)
        {
	  printf("*******************\n");
          socklen_t sin_size = sizeof(struct sockaddr_in);
          if((client_fd = accept(sock_fd,(struct sockaddr *)&remote_addr, &sin_size)) == -1)
          {
	     printf("wait\n");
             continue;
          }
	  printf("*******************\n");
	  printf("accept\n");
          if(!fork())
          {
        /*     if(send(client_fd,"Hello, you are connected,SB\n",26,0) == -1)
             {
	        printf("send\n");
                close(client_fd);
                exit(0);
             }
	*/
	while(1)
	{
	 if((recvbytes = recv(client_fd, &buf, sizeof(info), 0)) == -1)
        {
         // printf("cannot recv\n");
        }
	   else if(recvbytes > 0)
	   printf("addr = %s,time = %s,mac = %s\n",buf.addr,buf.time,buf.mac);
	 //   printf("board_id = %d, board_name = %s,board_version = %s,macaddr = %s,time = %s,event = %s\n",((info*)buf)->board_id,((info*)buf)->board_name,((info*)buf)->board_version,((info*)buf)->macaddr,((info*)buf)->time,((info*)buf)->event);
          }
	}
	}

} 
