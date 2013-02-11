#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/timeb.h> 
#include <sys/socket.h>
#include <fcntl.h>
#include "relay2server.h"
#include <limits.h>
#include <sys/stat.h>
#include <signal.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <limits.h>
#include <sys/stat.h>
#include <signal.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include "queue.h"

#define LOGFILE_ON
/*
 * #define TERMINAL_ON
 * */

#define DEFAULT_LOG_MAX_LINES 20000
#define DEFAULT_INQUIRY_LEN 5 
#define DEFAULT_LOG_FILE "btscan.out"
extern infoQueue *infoQ;
extern infoQueue *unSendQ;
 
int maxLoglines,querytime;
char *logname;

void config(int *maxLoglines, int *inquiry_len,char **logname)
{
        char *env;
        env = getenv("LOG_MAX_LINES");
        *maxLoglines = string2int(env);
        if(0 == *maxLoglines)
        {
                *maxLoglines =  DEFAULT_LOG_MAX_LINES;
        }
        env = getenv("INQUIRY_LEN");
        *inquiry_len = string2int(env);
        if(0 == *inquiry_len)
        {
                *inquiry_len = DEFAULT_INQUIRY_LEN;
        }
        *logname = getenv("LOGFILE");
        if(!(*logname))
        {
                *logname = DEFAULT_LOG_FILE;
		printf("logname %s\n",*logname);
        }

}
int     string2int(const char *env)
{
        int value;
        if(env)
        {
                   char* convTest = 0;
                   value = strtol(env, &convTest, 10);

        }
        else
        {
                  value  = 0;
        }
        return value;

}

void logHandler(int *line,char current_time[],FILE **logFile)
{

               char old_file_name[40];

               if (*line  >= maxLoglines)
               {

                        sprintf(old_file_name,"%s%s",logname,current_time);
                        rename(logname, old_file_name);
                        fclose(*logFile);
                        *logFile=fopen(logname,"w");
                        *line = 0;

               }

}

void getEth0Mac(char mac[])
{
    struct ifreq ifreq;
    int sock = 0;
   // char mac[32] = "";


    sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        perror("error sock");
        return ;
    }

    strcpy(ifreq.ifr_name,"eth0");
    if(ioctl(sock,SIOCGIFHWADDR,&ifreq) < 0)
    {
        perror("error ioctl");
        return ;
    }

    int i = 0;
    for(i = 0; i < 6; i++){
        sprintf(mac+3*i, "%02X:", (unsigned char)ifreq.ifr_hwaddr.sa_data[i]);
    }
    mac[strlen(mac) - 1] = 0;
    printf("MAC: %s\n", mac);
                
}


int main(int argc, char **argv)
{
    inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags,loglines,socket_fd;
    int i;
    int first_check_flag=0;
    char addr[19] = { 0 };
    char name[248] = { 0 };
    char current_time[50];
    
    info data;
    pthread_t pid;
    pthread_attr_t attr;
    pthread_mutex_t mutex;
    getEth0Mac(data.mac);
    pthread_create(&pid,NULL,relay2server,NULL);
    pthread_mutex_init(&queueMutex,NULL);
    InitQueue(& infoQ);
    InitQueue(&unSendQ);
    sleep(2);

        char *server_addr = getenv("SERVER_ADDRESS");
        if(!server_addr)
        {
           printf("no remote server address,exit thread");
        }
   socketopen(0,&socket_fd,server_addr);
    dev_id = hci_get_route(NULL);
    sock = hci_open_dev( dev_id );
    if (dev_id < 0 || sock < 0) {
        perror("opening socket");
        exit(1);
    }

   // len  = 7;
    max_rsp = 255;
    loglines = 0;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

        time_t now;
        struct tm *timenow;

#ifdef LOGFILE_ON
	config(&maxLoglines,&len,&logname);
        FILE *logFile;
        logFile=fopen(logname,"w");
#endif
 
                
    while (1){
        time(&now);
        timenow = localtime(&now);
        sprintf(current_time, "%d%02d%02d%02d%02d%02d",
            timenow->tm_year+1900, timenow->tm_mon+1, timenow->tm_mday, timenow->tm_hour, timenow->tm_min, timenow->tm_sec);

#ifdef TERMINAL_ON      
        printf("inquiry starts: %s",asctime(timenow));
#endif
#ifdef LOGFILE_ON       
        fprintf(logFile,"inquiry starts: %s",asctime(timenow));
#endif
        num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
        if( num_rsp < 0 ) perror("hci_inquiry");
	 else
	 {
	 	if(first_check_flag ==0)
	 	{
	 		first_check_flag = 1;
			mkfifo("fifo","0777");
	 	}
	 }
	 
	 
        for (i = 0; i < num_rsp; i++) {
                ba2str(&(ii+i)->bdaddr, addr);       
        	  memcpy(data.addr,addr,sizeof(addr));
		  //memcpy(data.time,current_time,sizeof(current_time));
		  memcpy(data.time,asctime(timenow),sizeof(data.time));
		  int res = EnQueue(infoQ, data);
		  if(!res)
		  {
			printf("The infoQueue is full \n");
		   }			
	
#ifdef TERMINAL_ON      
                        printf("%s  %s\n", addr, name);
#endif
#ifdef LOGFILE_ON      
			loglines++; 
                        fprintf(logFile, "%s  %s\n", addr, name);
	
#endif                  
            }

#ifdef TERMINAL_ON      
                        printf("%d devices found.\n",num_rsp);
                        printf("---------------------------------------\n");
                        fflush(stdout);
#endif
#ifdef LOGFILE_ON       
			   loglines++;
                        fprintf(logFile,"%d devices found.\n",num_rsp);
                        fprintf(logFile,"---------------------------------------\n");
                        fflush(logFile);
			  logHandler(&loglines,current_time,&logFile);
			
#endif
                }
    
#ifdef LOGFILE_ON       
                        fclose(logFile);
#endif
    free( ii );
    close( sock );
    return 0;
}
