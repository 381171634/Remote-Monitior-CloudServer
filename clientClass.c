/*
 ============================================================================
 Name        : clientClass.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : 客户端类
 ============================================================================
 */

#include "includes.h"

/*============================================================================
 客户端类初始化
 cc ：客户端指针
 s_scb_nbr ：套接字控制块序号
 ============================================================================*/
static int api_cc_init(clientClassTypedef *cc,int s_scb_nbr)
{
    memset(cc,0,sizeof(clientClassTypedef));
    cc->s_scb_nbr = s_scb_nbr;
    cc->socket    = s_scb[s_scb_nbr].socket_id;
    api_DB.init(&cc->db);
    api_procComm.procInit(&cc->recvBuf);

    return 1;
}

/*============================================================================
 客户端类去初始化
 cc ：客户端指针
 ============================================================================*/
static int api_cc_deinit(clientClassTypedef *cc)
{
    api_DB.deinit(cc->db);

    return 1;
}

/*============================================================================
 客户端类api接口注册
 ============================================================================*/
api_clientClassTypedef api_cc = {
    .init = api_cc_init,
    .deinit = api_cc_deinit
};