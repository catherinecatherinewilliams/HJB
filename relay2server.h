#define FIFO_NAME "/test/myfifo"
int socketopen(int type, int* sock_fd,char *server_addr);
int createfifo(char* fifoname);
int read2fifo(char *fifoname, char* buffer);
void run(int socket_fd,int type,char *server_address);
void *relay2server();

