/*
 ============================================================================
 Name        : main.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : 主程序
 ============================================================================
 */

#include "includes.h"

int main()
{
	int s_scb_nbr = 0;
	struct sockaddr_in client;
	socklen_t len = sizeof(client);

	//createDbForTest();
	//return;

	//建立服务器 开始监听
	int listen_sock = server_init("0.0.0.0",8888);

	if(listen_sock < 0)
	{
		DBG_PRT("server init fail\n");
		return 0;
	}

	while(1)
	{
		//检查是否还有可用套接字
		for(s_scb_nbr = 0;s_scb_nbr < SOCKET_MAX_NUM; s_scb_nbr++)
		{
			if(s_scb[s_scb_nbr].socket_id < 0)
			{
				break;
			}
		}

		if(s_scb_nbr < SOCKET_MAX_NUM)
		{
			//接入client
			s_scb[s_scb_nbr].socket_id = accept(listen_sock,(struct sockaddr*)&client,&len);
			if(s_scb[s_scb_nbr].socket_id < 0)
			{
				//DBG_PRT("accept error\n");
				s_scb[s_scb_nbr].socket_id = -1;
				memset(&s_scb[s_scb_nbr].client_info,0,sizeof(struct sockaddr_in));
			}
			else
			{
				connect_in(s_scb_nbr,&client);
			}
		}
		else
		{
			//DBG_PRT("socket connected FULL\n");
		}

	}

}