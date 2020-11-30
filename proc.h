#ifndef _PROC_H
#define _PROC_H

#include "common.h"

#define PROC_BUF_LEN            2048    //协议环形缓存长度
#define PROC_CONTENT_LEN        512     //协议内容长度
#define PROC_SOCKET_BUF_LEN     512     //协议套接字读出最大长度


typedef enum{
    PROC_FIND_1ST_HEAD = 0,
    PROC_FIND_2ND_HEAD,
    PROC_GET_LEN_L,
    PROC_GET_LEN_H,
    PROC_GET_DATA,
    PROC_CHECK_SUM
}ProcStateTypedef;//协议状态机

typedef struct{
    int isLocked;                         //上锁
    uint8_t buf[PROC_BUF_LEN];            //环形缓存
    uint8_t contentBuf[PROC_CONTENT_LEN]; //协议内容缓存
    uint16_t proclen;                     //协议长度
    uint16_t pWrite_contentBuf;           //协议内容写指针
    uint16_t pRead;                       //环形缓存读指针
    uint16_t pWrite;                      //环形缓存写指针
    uint8_t procState;                    //协议状态机
}recvChannelfTypedef;

typedef struct{
    void    (*procInit)(recvChannelfTypedef *procRBuf);                  
    int     (*readSocket)(int socket_id,recvChannelfTypedef *procRBuf);
    int     (*getProc)(recvChannelfTypedef *procRBuf);
    int     (*makeAproc)(uint8_t *pSrc,uint16_t len);
}api_procCommunicateTypedef;//协议通讯总方法，以传入的recvChannelfTypedef区分设备

extern api_procCommunicateTypedef api_procComm;
#endif