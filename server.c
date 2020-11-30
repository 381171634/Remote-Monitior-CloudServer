/*
 ============================================================================
 Name        : server.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : 服务器
 ============================================================================
 */

#include "includes.h"

static unsigned int SocketUsed = 0;	//使用中的套接字个数
static pthread_mutex_t mutex;		//线程互斥锁
SOCKET_SCB s_scb[SOCKET_MAX_NUM];	//套接字块


/*============================================================================
 互斥锁初始化
 ============================================================================*/
static void mutext_init(pthread_mutex_t *mutex)
{
	pthread_mutex_init(mutex,NULL);
}

/*============================================================================
 套接字控制块初始化
 ============================================================================*/
static void socketSCBInit(void)
{
	int j;
	for(j = 0;j < SOCKET_MAX_NUM; j++)
	{
		s_scb[j].socket_id = -1;
		memset(&s_scb[j].client_info,0,sizeof(struct sockaddr_in));
	}
}

/*============================================================================
 服务器初始化
 port:端口
 ============================================================================*/
int server_init(char *ip,int port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	int bindnum;

	struct timeval recvTimeout = {60,0};
	struct timeval sendTimeout = {10,0};

	socketSCBInit();
	mutext_init(&mutex);

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
		DBG_PRT("bind fail\n");
		return bindnum;
	}

	if(listen(sock,SOCKET_MAX_NUM) < 0)
	{
		DBG_PRT("listen fail\n");
		return -1;
	}
	else
	{
		DBG_PRT("listen ing ....\n");
	}
	return sock;

}


/*============================================================================
 套接字线程，每个套接字分配一个线程
 arg：传参数，传的是套接字块序号
 ============================================================================*/
void *thread_socket(void *arg)
{

	int s_scb_nbr = arg;
	int socket_id = s_scb[s_scb_nbr].socket_id;
	//进入应用层
    app_process(s_scb_nbr);
	DBG_PRT("app_process will exit3\n");
	//断开链接
	connect_out(s_scb_nbr,socket_id);
	
}

/*============================================================================
 客户端接入
 s_scb_nbr：套接字块序号
 clinet：客户端信息
 ============================================================================*/
void connect_in(int s_scb_nbr,struct sockaddr* client)
{
	pthread_t id;

	pthread_mutex_lock(&mutex);
	SocketUsed++;
	memcpy(&s_scb[s_scb_nbr].client_info,client,sizeof(struct sockaddr));
	pthread_mutex_unlock(&mutex);

	DBG_PRT("new dev join -> %d %s\n",s_scb[s_scb_nbr].socket_id,inet_ntoa(s_scb[s_scb_nbr].client_info.sin_addr));
	DBG_PRT("avillable socket_id :%d\n",SOCKET_MAX_NUM - SocketUsed);
	pthread_create(&id,NULL,thread_socket,(void *)s_scb_nbr);
	pthread_detach(id);
}

/*============================================================================
 客户端退出
 s_scb_nbr：套接字块序号
 clinet：客户端信息
 ============================================================================*/
void connect_out(int s_scb_nbr,int socket_id)
{
	DBG_PRT("dev quit -> %d %s\n",s_scb[s_scb_nbr].socket_id,inet_ntoa(s_scb[s_scb_nbr].client_info.sin_addr));
	pthread_mutex_lock(&mutex);
	s_scb[s_scb_nbr].socket_id = -1;
	memset(&s_scb[s_scb_nbr].client_info,0,sizeof(struct sockaddr_in));
	SocketUsed--;
	pthread_mutex_unlock(&mutex);

	DBG_PRT("avillable socket_id :%d\n",SOCKET_MAX_NUM - SocketUsed);
	close(socket_id);
}
