#include "includes.h"

unsigned int SocketUsed = 0;								//使用中的套接字个数
SOCKET_SCB s_scb[SOCKET_MAX_NUM];							//套接字块
pthread_mutex_t mutex;										//线程互斥锁

//互斥锁初始化
void mutext_init(pthread_mutex_t *mutex)
{
	pthread_mutex_init(mutex,NULL);
}

//套接字池初始化
void SocketSCBInit(void)
{
	int j;
	for(j = 0;j < SOCKET_MAX_NUM; j++)
	{
		s_scb[j].socket_id = -1;
		memset(&s_scb[j].client_info,0,sizeof(struct sockaddr_in));
	}
}

//服务器初始化
int server_init(char *ip,int port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	int bindnum;

	struct timeval recvTimeout = {60,0};
	struct timeval sendTimeout = {10,0};

	if(sock < 0)
	{
		return sock;
	}

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);

	setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(void *)&recvTimeout,sizeof(struct timeval));
	setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(void *)&sendTimeout,sizeof(struct timeval));

	bindnum = bind(sock,(struct sockaddr *)&local,sizeof(local));
	if(bindnum < 0)
	{
		printf("bind fail\n");
		return bindnum;
	}

	if(listen(sock,SOCKET_MAX_NUM) < 0)
	{
		printf("listen fail\n");
		return -1;
	}
	else
	{
		printf("listen ing ....\n");
	}
	return sock;

}


//线程
void *thread_socket(void *arg)
{

	int SocketNbr = arg;
	int socket_id = s_scb[SocketNbr].socket_id;
	
    app_process(SocketNbr);
	printf("app_process will exit3\n");
	connect_out(SocketNbr,socket_id);
	
}

//client接入
void connect_in(int SocketNbr,struct sockaddr* client)
{
	pthread_t id;

	pthread_mutex_lock(&mutex);
	SocketUsed++;
	memcpy(&s_scb[SocketNbr].client_info,client,sizeof(struct sockaddr));
	pthread_mutex_unlock(&mutex);

	printf("new dev join -> %d %s\n",s_scb[SocketNbr].socket_id,inet_ntoa(s_scb[SocketNbr].client_info.sin_addr));
	printf("avillable socket_id :%d\n",SOCKET_MAX_NUM - SocketUsed);
	pthread_create(&id,NULL,thread_socket,(void *)SocketNbr);
	pthread_detach(id);
}

//client推出
void connect_out(int SocketNbr,int socket_id)
{
	printf("dev quit -> %d %s\n",s_scb[SocketNbr].socket_id,inet_ntoa(s_scb[SocketNbr].client_info.sin_addr));
	pthread_mutex_lock(&mutex);
	s_scb[SocketNbr].socket_id = -1;
	memset(&s_scb[SocketNbr].client_info,0,sizeof(struct sockaddr_in));
	SocketUsed--;
	pthread_mutex_unlock(&mutex);

	printf("avillable socket_id :%d\n",SOCKET_MAX_NUM - SocketUsed);
	close(socket_id);
}
