/*
 ============================================================================
 Name        : app.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : 服务器应用层
 ============================================================================
 */

#include "includes.h"

/*============================================================================
 收到报文后打印 用于服务器调试
 cc ：客户端指针
 ============================================================================*/
void showRBcontent(clientClassTypedef *cc)
{
    DBG_PRT("get from s_scb_nbr %d:",cc->s_scb_nbr);
    for(int i = 0;i < cc->recvBuf.pWrite_contentBuf;i++)
    {
        printf("%02x ",cc->recvBuf.contentBuf[i]);
        
    }

    printf("\n");
}


/*============================================================================
 协议处理方法
 ============================================================================*/

/*============================================================================
 设备上线处理
 cc ：客户端指针
 ============================================================================*/
static int deal_dev_online(clientClassTypedef *cc)
{
    int res = FALSE;
    char ackBuf[PROC_CONTENT_LEN] = {0};
    unsigned short len_t = 4;
    uint8_t ackLen = 0;
    uint8_t *p = &cc->recvBuf.contentBuf[1];
    recordTypedef record;
    memset((void *)&record,0,sizeof(recordTypedef));

    showRBcontent(cc);

    if((1 + 4 + 16) == cc->recvBuf.pWrite_contentBuf)
    {
        memcpy((void *)&record.sData.timeTick,p,sizeof(int));
        p += sizeof(int);
        memcpy((void *)record.dev_id,p,16);

        DBG_PRT("dev_time = %d\ndev_id = %s\n",record.sData.timeTick,record.dev_id);
        //operate database
        res = api_DB.devOnline(&record,cc->db);
        if(res == TRUE)
        {
            //ack;
            ackBuf[len_t] = cc->recvBuf.contentBuf[0];
            ackLen = api_procComm.makeAproc(ackBuf,1);
            send(cc->socket,ackBuf,ackLen,0);
        }

    }
    
    return res ;
}

/*============================================================================
 设备心跳
 cc ：客户端指针
 ============================================================================*/
static int deal_dev_heart(clientClassTypedef *cc)
{
    int res = FALSE;

    showRBcontent(cc);

    DBG_PRT("dev heart beat\n");

    res = TRUE;

    return res ;
}

/*============================================================================
 设备上报信息
 cc ：客户端指针
 ============================================================================*/
static int deal_dev_publish(clientClassTypedef *cc)
{
    int res = FALSE;
    char ackBuf[PROC_CONTENT_LEN] = {0};
    unsigned short len_t = 4;
    uint8_t ackLen = 0;
    uint8_t *p = &cc->recvBuf.contentBuf[1];
    recordTypedef record;
    memset((void *)&record,0,sizeof(recordTypedef));

    showRBcontent(cc);

    if((1 + 4 + 16 + sizeof(SampleDataTypedef)) == cc->recvBuf.pWrite_contentBuf)
    {
        memcpy((void *)&record.sData.timeTick,p,sizeof(int));
        p += sizeof(int);
        memcpy((void *)record.dev_id,p,16);
        p += 16;
        record.dev_id[16] = 0;
        memcpy((void *)&record.sData,p,sizeof(SampleDataTypedef));

        DBG_PRT("dev_time = %d\ndev_id = %s\n",record.sData.timeTick,record.dev_id);
        DBG_PRT("tempture:%.3f\n",     (float)record.sData.tempture / 1000);
        DBG_PRT("humidity:%.3f\n",     (float)record.sData.humidity / 1000);
        DBG_PRT("HCHO:%.3f\n",         (float)record.sData.HCHO / 1000);
        DBG_PRT("CO2:%.3f\n",          (float)record.sData.CO2 / 1000);
        DBG_PRT("cellVoltage:%.3f\n",  (float)record.sData.cellVoltage / 1000);
        //operate database
        res = api_DB.devPublish(&record,cc->db);
        if(res == TRUE)
        {
            //ack;
            ackBuf[len_t] = cc->recvBuf.contentBuf[0];
            ackLen = api_procComm.makeAproc(ackBuf,1);
            send(cc->socket,ackBuf,ackLen,0);
        }
        

    }
    
    return res ;
}

/*============================================================================
 手机上线
 cc ：客户端指针
 ============================================================================*/
static int deal_phone_online(clientClassTypedef *cc)
{
    int res = FALSE;
    int len_t = 4;
    uint8_t ackBuf[PROC_CONTENT_LEN] = {0};
    unsigned short ackLen = 0;
    uint8_t *p = &cc->recvBuf.contentBuf[1];
    recordTypedef record;
    memset((void *)&record,0,sizeof(recordTypedef));

    showRBcontent(cc);

    if((1 + 16) == cc->recvBuf.pWrite_contentBuf)
    {
        memcpy(record.dev_id,p,16);
        //operate database
        res = api_DB.phoneOnline(&record,cc->db);
        //数据库中无此设备
        if(res == FALSE)
        {
            ackBuf[len_t] = cc->recvBuf.contentBuf[0];
            len_t++;
            ackBuf[len_t] = 0;
            len_t++;

            ackLen = api_procComm.makeAproc(ackBuf,len_t - 4);
        } 
        //数据库中有此设备
        else
        {
            ackBuf[len_t] = cc->recvBuf.contentBuf[0];
            len_t++;
            ackBuf[len_t] = 1;
            len_t++;
            memcpy(ackBuf + len_t,&record.sData,sizeof(SampleDataTypedef));
            len_t+=sizeof(SampleDataTypedef);
            ackLen = api_procComm.makeAproc(ackBuf,len_t - 4);
        }
        
        send(cc->socket,ackBuf,ackLen,0);
    }

    return res;
}

/*============================================================================
 手机查询
 cc ：客户端指针
 ============================================================================*/
static int deal_phone_query(clientClassTypedef *cc)
{
    int res = FALSE;
    int len_t = 4;
    uint8_t ackBuf[PROC_CONTENT_LEN] = {0};
    unsigned short ackLen = 0;
    uint8_t *p = &cc->recvBuf.contentBuf[1];
    uint8_t dev_id[17] = {0};
    int startTick = 0;
    int endTick = 0;
    int recordSentCnt = 0;

    showRBcontent(cc);

    if((1 + 16 + 8) == cc->recvBuf.pWrite_contentBuf)
    {
        memcpy(dev_id,p,16);
        p += 16;
        memcpy((void *)&startTick,p,sizeof(int));
        p += 4;
        memcpy((void *)&endTick,p,sizeof(int));
        //operate database
        res = api_DB.phoneQuery(dev_id,startTick,endTick,&cc->pRecord,cc->db);
        if(res == TRUE)
        {
            if(cc->pRecord.pkgTotal == 1)
            {
                ackBuf[len_t++] = 0x04;
                ackBuf[len_t++] = (uint8_t)cc->pRecord.pkgTotal;
                ackBuf[len_t++] = (uint8_t)(++cc->pRecord.pkgSent);
                memcpy(ackBuf + len_t,&cc->pRecord.recordCnt,2);
                len_t += 2;
                memcpy(ackBuf + len_t,&cc->pRecord.recordBuf[cc->pRecord.pUpload],sizeof(SampleDataTypedef) * cc->pRecord.recordCnt);
                len_t += sizeof(SampleDataTypedef) * cc->pRecord.recordCnt;
                ackLen = api_procComm.makeAproc(ackBuf,len_t - 4);
                send(cc->socket,ackBuf,ackLen,0);

                free((void *)cc->pRecord.recordBuf);
                memset((void *)&cc->pRecord,0,sizeof(foundRecordsCacheTypedef));
 
            }
            else
            {
                //有后续帧的应答 第一包
                ackBuf[len_t++] = 0x14;
                recordSentCnt = (LEN_OF_ONE_PKG) / sizeof(SampleDataTypedef);
                DBG_PRT("recordSentCnt:%d\n",recordSentCnt);
                ackBuf[len_t++] = (uint8_t)cc->pRecord.pkgTotal;
                ackBuf[len_t++] = (uint8_t)(++cc->pRecord.pkgSent);
                memcpy(ackBuf + len_t,&recordSentCnt,2);
                len_t += 2;
                memcpy(ackBuf + len_t,&cc->pRecord.recordBuf[cc->pRecord.pUpload],sizeof(SampleDataTypedef) * recordSentCnt);
                len_t += sizeof(SampleDataTypedef) * recordSentCnt;
                cc->pRecord.pUpload += recordSentCnt;

                ackLen = api_procComm.makeAproc(ackBuf,len_t - 4);
                send(cc->socket,ackBuf,ackLen,0);
            }
        }
        else
        {
            //N条采样数据为0,表示没查到
            ackBuf[len_t++] = 0x04;
            memcpy(ackBuf + len_t,&cc->pRecord.recordCnt,2);
            len_t += 2;

            ackLen = api_procComm.makeAproc(ackBuf,len_t - 4);
            send(cc->socket,ackBuf,ackLen,0);
        }
    }

    return res;
}

/*============================================================================
 手机后续帧应答处理
 cc ：客户端指针
 ============================================================================*/
static int deal_phone_follow_ack(clientClassTypedef *cc)
{
    int res = FALSE;
    int len_t = 4;
    uint8_t ackBuf[PROC_CONTENT_LEN] = {0};
    unsigned short ackLen = 0;
    int recordSentCnt = 0;

    if(cc->pRecord.pkgTotal - cc->pRecord.pkgSent == 1)
    {
        recordSentCnt = cc->pRecord.lastPkgCnt;
        DBG_PRT("cc->pRecord.pkgSent:%d\n",cc->pRecord.pkgSent);
        ackBuf[len_t++] = 0x04;
        ackBuf[len_t++] = (uint8_t)cc->pRecord.pkgTotal;
        ackBuf[len_t++] = (uint8_t)(++cc->pRecord.pkgSent);
        memcpy(ackBuf + len_t,&recordSentCnt,2);
        len_t += 2;
        memcpy(ackBuf + len_t,&cc->pRecord.recordBuf[cc->pRecord.pUpload],sizeof(SampleDataTypedef) * recordSentCnt);
        len_t += sizeof(SampleDataTypedef) * recordSentCnt;
        ackLen = api_procComm.makeAproc(ackBuf,len_t - 4);
        send(cc->socket,ackBuf,ackLen,0);

        free((void *)cc->pRecord.recordBuf);
        memset((void *)&cc->pRecord,0,sizeof(foundRecordsCacheTypedef));

    }
    else if(cc->pRecord.pkgTotal - cc->pRecord.pkgSent > 1)
    {
        DBG_PRT("cc->pRecord.pkgSent:%d\n",cc->pRecord.pkgSent);

        ackBuf[len_t++] = 0x14;
        recordSentCnt = (LEN_OF_ONE_PKG) / sizeof(SampleDataTypedef);
        ackBuf[len_t++] = (uint8_t)cc->pRecord.pkgTotal;
        ackBuf[len_t++] = (uint8_t)(++cc->pRecord.pkgSent);
        memcpy(ackBuf + len_t,&recordSentCnt,2);
        len_t += 2;
        memcpy(ackBuf + len_t,&cc->pRecord.recordBuf[cc->pRecord.pUpload],sizeof(SampleDataTypedef) * recordSentCnt);
        len_t += sizeof(SampleDataTypedef) * recordSentCnt;
        cc->pRecord.pUpload += recordSentCnt;

        ackLen = api_procComm.makeAproc(ackBuf,len_t - 4);
        send(cc->socket,ackBuf,ackLen,0);
    }

    return res;
}

/*============================================================================
 协议脚本注册
 ============================================================================*/

static ProcScriptTypedef procScript[] = {
    {CTRL_DEV_ONLINE,   deal_dev_online},
    {CTRL_DEV_HEART,    deal_dev_heart},
    {CTRL_DEV_PUBLISH,  deal_dev_publish},
    {CTRL_PHONE_ONLINE, deal_phone_online},
    {CTRL_PHONE_QUERY,  deal_phone_query},
    {CTRL_PHONE_FOLLOW_ACK,deal_phone_follow_ack},
};

/*============================================================================
 app层流程
 ============================================================================*/
void app_process(int s_scb_nbr)
{
    int res,res_getProc,i;
    clientClassTypedef cc;
    api_cc.init(&cc,s_scb_nbr);
    
    while(1)
    {
        //从socket接受数据
        res = api_procComm.readSocket(cc.socket,&cc.recvBuf); 
        
        //超时或错误，退出线程，退出tcp链接
        if(res <= 0)
        {
            DBG_PRT("app_process will exit\n");
            if((uint8_t *)cc.pRecord.recordBuf != 0)
            {
                DBG_PRT("free pRecord cause pRecord=%x\n",cc.pRecord.recordBuf);
                free((void *)cc.pRecord.recordBuf);
                memset((void *)&cc.pRecord,0,sizeof(foundRecordsCacheTypedef));
            }

            api_cc.deinit(&cc);

            break;
        }

        //搜报文
        do
        {
            res_getProc = api_procComm.getProc(&cc.recvBuf);
            if(res_getProc == 1)
            {
                //如果不是后续帧应答
                if(cc.recvBuf.contentBuf[0] != CTRL_PHONE_FOLLOW_ACK)
                {
                    //如果后续帧缓存区已经申请，则清空
                    if(cc.pRecord.recordBuf != 0)
                    {
                        free(cc.pRecord.recordBuf);
                        memset((void *)&cc.pRecord,0,sizeof(foundRecordsCacheTypedef));
                    }
                }
                //匹配脚本
                for(i = 0;i < CTRL_END;i++)
                {
                    //控制字匹配
                    if(cc.recvBuf.contentBuf[0] == procScript[i].procID)
                    {
                        //执行协议处理
                        res = procScript[i].procDeal(&cc);

                        //执行后处理
                        memset((void *)cc.recvBuf.contentBuf,0,sizeof(PROC_CONTENT_LEN));
                        cc.recvBuf.isLocked = 0;
                        break;
                    }
                }
            }
        }while(res_getProc == 1);
        
    }

    return;
}
