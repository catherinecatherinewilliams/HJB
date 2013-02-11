#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include "relay2server.h"
#include "queue.h"
#define CLIENT 0
#define SERVER 1
#define MAXDATASIZE 100
#define SERVPORT 3333 /*lisen port*/
#define BACKLOG 10 /*maxisum connection*/
#define false 0
#define true 1
#define BUFFER_SIZE PIPE_BUF  
#define TEN_MEG (1024 * 1024 * 10)
//extern infoQueue *infoQueue;
typedef enum {
CONNECTION_UP = 0,
CONNECTION_DOWN = 1
} CONNECT_STATE;


extern infoQueue *infoQ;
extern infoQueue  *unSendQ;
int conState;

int socketopen(int type, int* sock_fd,char *server_addr)
{
   struct sockaddr_in  servAddr;
   struct hostent *host;
   int flags;
   if (type == SERVER)
   {
       if ( (*sock_fd = socket (AF_INET, SOCK_STREAM, 0) ) < 0)
       {
           // Socket creating failed.
          printf("socket_open() Socket creating failed. Exit: false\n");
          return false;
       }

       servAddr.sin_family = AF_INET;
       servAddr.sin_addr.s_addr = htonl (INADDR_ANY);
       servAddr.sin_port = htons (SERVPORT);
      // setsockopt( sock_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&ival, sizeof (ival) ); // ignore if it failed... 

         if (bind (*sock_fd, (struct sockaddr*) &servAddr, sizeof (servAddr) ) < 0)
         {
             // Binding failed.
             printf("socketopen() Binding failed. Exit: false\n");
             return false;
         }

        if (listen (*sock_fd, BACKLOG) < 0)
        {
                 // Listen failed.
                 printf("socket_open() Listen failed. Exit: false\n");
                 return false;
        }
   }
   else if (type == CLIENT)
   {
       if((host = gethostbyname(server_addr)) == NULL)
       { 
	}        
       if ((*sock_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
       {
                // Socket creating failed.
                printf("socket_open()Socket creating failed. Exit: false\n");
                return false;
       }
      flags = fcntl(*sock_fd, F_GETFL, 0);                    
      fcntl(*sock_fd, F_SETFL, flags | O_NONBLOCK); 
      servAddr.sin_family = AF_INET;
      servAddr.sin_addr = *((struct in_addr *)host->h_addr);
      servAddr.sin_port = htons (SERVPORT);
      connect(*sock_fd,(struct sockaddr *)&servAddr, sizeof(struct sockaddr_in));
	 printf("connecting ***********************\n");
      // client side needs not to be accepted
      
      return true;
   }	
}


    
void run(int socket_fd,int type,char *server_address)
{
	info data;
	int ismessage;
	int select_counter = 0;
	int fileCounter =1;
	int unSendFlag = 0;
	fd_set r_fds ;
       fd_set w_fds ;
	int lastdata_flag = 0;
	info lastdata;
	struct timeval timeRec;
	struct timeval *tmptimeRec = &timeRec;
	FILE *dataFile;
	char old_dataFile_name[40];
	char current_time[50];
	 time_t now;
        struct tm *timenow;

      
	socketopen(type,&socket_fd,server_address);
      	FD_ZERO(&w_fds);
        FD_SET(socket_fd, &w_fds);
	printf("run***************\n");
	while(1)
	{
   	 	 timeRec.tv_usec= 2;
		 timeRec.tv_sec = 2;
		  int ret = select(FD_SETSIZE, NULL, &w_fds, NULL, tmptimeRec);
       	 if(0 > ret )
       	  {          
     			printf("Error in 'Select'?! Trying to continue...\n");
			continue;
        	   }
	
		 if(conState == CONNECTION_UP)
		 {
		 	
		 	if(!lastdata_flag)
			{
				if(!QueueEmpty(infoQ))
				{
					DeQueue(infoQ,&data);
					printf("DeQueue data.addr = %s, data.time = %s,data.mac = %s\n",data.addr,data.time,data.mac);
				}
				else
				{
						
		 				if(QueueEmpty(unSendQ))
		 				{
		 					if(unSendFlag)
		 					{
		 						int i;
								sprintf(old_dataFile_name,"%s%d","dataFile_unsend_",fileCounter);
								dataFile=fopen(old_dataFile_name,"rb");
								for(i = 0; i< QUEUE_MAX_SIZE; i++)
								{
									fread(&data,sizeof(info),1,dataFile);
									EnQueue(unSendQ, data);
								}
								fclose(dataFile);
								remove(old_dataFile_name);
								if(fileCounter == 1)
		 						unSendFlag = 0;
								else
								fileCounter--;
								
		 					}
							 continue;
		 				}
					     	else
						{
							DeQueue(unSendQ,&data);
							printf("DeQueue unsend data.addr = %s,data.mac = %s\n",data.addr,data.time,data.mac);
						}

		 		}
		 	}
			else
			data = lastdata;
		 }
		 
		    int len = QueueLength(infoQ);
		       
		    if (len == QUEUE_MAX_SIZE-1 ||len == -1)
		    {
		 		printf("Queue is FULL*************\n");
				sprintf(old_dataFile_name,"%s%d","dataFile_unsend_",fileCounter);
				dataFile=fopen(old_dataFile_name,"wb");
				fileCounter++;
				unSendFlag =1;
				int i;
				for(i = 0;i < QUEUE_MAX_SIZE;i++)
				{
					DeQueue(infoQ,&data);
					fwrite(&data,sizeof(info),1,dataFile);
				}
				fclose(dataFile);
				/*
				time(&now);
       			timenow = localtime(&now);
       			sprintf(current_time, "_%d%02d%02d%02d%02d%02d",
            timenow->tm_year+1900, timenow->tm_mon+1, timenow->tm_mday, timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
            			*/
            			/*
				sprintf(old_dataFile_name,"%s%d","dataFile_unsend_",fileCounter);
                        	rename("dataFile", old_dataFile_name);
                       	fclose(dataFile);
                        	dataFile=fopen("dataFile","w");
				*/
		    }
        	 if(FD_ISSET(socket_fd, &w_fds))
        	  {	


            			if(send(socket_fd,&data,sizeof(info),MSG_DONTWAIT) == -1)  
            			{	
					printf(" send failed,need consider peer down\n");
					lastdata = data;
					lastdata_flag = 1;
					conState == CONNECTION_DOWN;				
           
            			}       
				else
				{	lastdata_flag = 0;
					printf("send data.addr =%s,data.time = %s,data.mac = %s\n",data.addr,data.time,data.mac);
				}
	
                 }
		 else
		 {
			select_counter++;
			if(select_counter > 20)//we think the network failed,reopen socket
			{
				close(socket_fd);
				socketopen(type, &socket_fd,server_address);
				conState = CONNECTION_UP;
				FD_ZERO(&w_fds);
        			FD_SET(socket_fd, &w_fds);
				select_counter = 0;
				printf(" socketopen**************\n",select_counter);
				
			}
		 }
	}
		
}
	 
        

 
 void *relay2server()
 {	
	int type = CLIENT;
	int socket_fd;
	char *server_addr = getenv("SERVER_ADDRESS");
	if(!server_addr)
	{
	   printf("no remote server address,exit thread");
	}
	run(socket_fd,type,server_addr);
}

	
	



