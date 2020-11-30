#include "server.h"
#include "proc.h"
#include "app.h"
#include "database.h"

int main()
{
	int SocketNbr = 0;
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	
	SocketSCBInit();
	mutext_init(&mutex);

	//createDbForTest();
	//return;

	//建立服务器 开始监听
	int listen_sock = server_init("0.0.0.0",8888);

	if(listen_sock < 0)
	{
		printf("server init fail\n");
		return 0;
	}

	while(1)
	{
		//检查是否还有可用套接字
		for(SocketNbr = 0;SocketNbr < SOCKET_MAX_NUM; SocketNbr++)
		{
			if(s_scb[SocketNbr].socket_id < 0)
			{
				break;
			}
		}

		if(SocketNbr < SOCKET_MAX_NUM)
		{
			//接入client
			s_scb[SocketNbr].socket_id = accept(listen_sock,(struct sockaddr*)&client,&len);
			if(s_scb[SocketNbr].socket_id < 0)
			{
				//printf("accept error\n");
				s_scb[SocketNbr].socket_id = -1;
				memset(&s_scb[SocketNbr].client_info,0,sizeof(struct sockaddr_in));
			}
			else
			{
				connect_in(SocketNbr,&client);
			}
		}
		else
		{
			//printf("socket connected FULL\n");
		}

	}

}