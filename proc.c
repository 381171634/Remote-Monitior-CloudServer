/*
 ============================================================================
 Name        : proc.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : 协议处理
            head_low    head_high   len_l len_h ctrl    data    sum_check
              0xa5         0x5a       xx    xx   xx    xx...xx     xx
 ============================================================================
 */

#include "includes.h"

/*============================================================================
 协议通道初始化
 procRBuf：通道指针
 ============================================================================*/
static void proc_procInit(recvChannelfTypedef *procRBuf)
{
    memset((void *)procRBuf,0,sizeof(recvChannelfTypedef));
    procRBuf->procState = PROC_FIND_1ST_HEAD;
    procRBuf->isLocked = 0;
}

/*============================================================================
 从套接字读数据
 procRBuf：通道指针
 return：读到的数据长度
 ============================================================================*/
static int proc_readSocket(int socket_id,recvChannelfTypedef *procBuf)
{
    int revlen,i;
    uint8_t buf[PROC_SOCKET_BUF_LEN] = {0};
    //阻塞
    revlen = recv(socket_id,buf,PROC_SOCKET_BUF_LEN,0);

    if(revlen > 0)
    {
        for(i = 0;i < revlen; i++)
        {
            procBuf->buf[(procBuf->pWrite++) % PROC_BUF_LEN] = buf[i];
        }
    }

    return revlen;
}

/*============================================================================
 重置协议状态机
 procRBuf：通道指针
 ============================================================================*/
static void rstProcState(recvChannelfTypedef *procRBuf)
{
    procRBuf->procState = PROC_FIND_1ST_HEAD;
}

/*============================================================================
 按协议搜报文
 procRBuf：通道指针
 return：0->未搜到  1->搜到
 ============================================================================*/
static int proc_getProc(recvChannelfTypedef *procRBuf)
{
    int res = 0;
    int i;
    uint8_t byte;
    uint8_t sum = 0;

    if(procRBuf->isLocked)
    {
        return res;
    }

    while(procRBuf->pRead - procRBuf->pWrite != 0)
    {
        byte = procRBuf->buf[(procRBuf->pRead++) % PROC_BUF_LEN];
        switch(procRBuf->procState)
        {
            case PROC_FIND_1ST_HEAD:
                if(byte == 0xa5)
                {
                    procRBuf->procState++;
                }
                break;
            case PROC_FIND_2ND_HEAD:
                if(byte == 0x5a)
                {
                    procRBuf->procState++;
                    procRBuf->proclen  = 0;
                }
                else
                {
                    rstProcState(procRBuf);
                }
                break;
            case PROC_GET_LEN_L:
                procRBuf->proclen &= ~(0xff);
                procRBuf->proclen |= byte;
                procRBuf->procState++;
                break;
            case PROC_GET_LEN_H:
                procRBuf->proclen &= ~(0xff00);
                procRBuf->proclen |= (byte << 8);
                if(procRBuf->proclen > PROC_CONTENT_LEN)
                {
                    rstProcState(procRBuf);
                }
                else
                {
                    procRBuf->pWrite_contentBuf = 0;
                    procRBuf->procState++;
                }
                break;
            case PROC_GET_DATA:
                if(procRBuf->pWrite_contentBuf < procRBuf->proclen)
                {
                    procRBuf->contentBuf[(procRBuf->pWrite_contentBuf) % PROC_CONTENT_LEN] = byte;
                }

                if(++procRBuf->pWrite_contentBuf >= procRBuf->proclen)
                {
                    procRBuf->procState++;
                }
                break;
            case PROC_CHECK_SUM:
                for(i = 0;i < procRBuf->proclen; i++)
                {
                    sum += procRBuf->contentBuf[i];
                }

                if(sum == byte)
                {
                    res = 1;
                    procRBuf->isLocked = 1;
                }

                rstProcState(procRBuf);
                break;
            default:
                rstProcState(procRBuf);
                break;
        }

        if(res == 1)
        {
            break;
        }
    }

    return res;
}

/*============================================================================
 协议组包
 pSrc：报文缓存指针
 len：控制字+数据域长度
 return：此包报文总长度
 ============================================================================*/
static int proc_makeAproc(uint8_t *pSrc,unsigned short len)
{
    int i;
    int p = 0;
    uint8_t *pContent = pSrc + 4;
    uint8_t sum = 0;

    pSrc[p++] = 0xa5;
    pSrc[p++] = 0x5a;
    memcpy((void *)(pSrc + p),&len,2);
    p += 2;
    p += len;
    for(i = 0; i< len; i++)
    {
        sum += pContent[i];
    }
    pSrc[p] = sum;

    return (len + 2 + 2 + 1);
}

/*============================================================================
 协议接口函数注册
 ============================================================================*/
api_procCommunicateTypedef api_procComm = {
    .procInit       = proc_procInit,
    .readSocket     = proc_readSocket,
    .getProc        = proc_getProc,
    .makeAproc      = proc_makeAproc
};
