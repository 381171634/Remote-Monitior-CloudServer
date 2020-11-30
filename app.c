#include "includes.h"

void showRBcontent(clientClassTypedef *cc)
{
    printf("get from socketNbr %d:",cc->SocketNbr);
    for(int i = 0;i < cc->recvBuf.pWrite_contentBuf;i++)
    {
        printf("%02x ",cc->recvBuf.contentBuf[i]);
        
    }

    printf("\n");
}

//==================================================================
//协议处理方法
//==================================================================

static int deal_dev_online(clientClassTypedef *cc)
{
    int res = 0;
    char ackBuf[PROC_CONTENT_LEN] = {0};
    unsigned short len_t = 4;
    unsigned char ackLen = 0;
    unsigned char *p = &cc->recvBuf.contentBuf[1];
    recordTypedef record;
    memset((void *)&record,0,sizeof(recordTypedef));

    showRBcontent(cc);

    if((1 + 4 + 16) == cc->recvBuf.pWrite_contentBuf)
    {
        memcpy((void *)&record.sData.timeTick,p,sizeof(int));
        p += sizeof(int);
        memcpy((void *)record.dev_id,p,16);

        printf("dev_time = %d\ndev_id = %s\n",record.sData.timeTick,record.dev_id);
        //operate database
        res = api_DB.devOnline(&record,cc->db);
        if(res == SQLITE_OK)
        {
            res = 1;
            //ack;
            ackBuf[len_t] = cc->recvBuf.contentBuf[0];
            ackLen = api_procComm.makeAproc(ackBuf,1);
            send(cc->socket,ackBuf,ackLen,0);
        }
        else
        {
            res = 0;
        }  
    }
    
    return res ;
}

static int deal_dev_heart(clientClassTypedef *cc)
{
    int res = 0;

    showRBcontent(cc);

    printf("dev heart beat\n");

    res = 1;

    return res ;
}

static int deal_dev_publish(clientClassTypedef *cc)
{
    int res = 0;
    char ackBuf[PROC_CONTENT_LEN] = {0};
    unsigned short len_t = 4;
    unsigned char ackLen = 0;
    unsigned char *p = &cc->recvBuf.contentBuf[1];
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

        printf("dev_time = %d\ndev_id = %s\n",record.sData.timeTick,record.dev_id);
        printf("tempture:%.3f\n",     (float)record.sData.tempture / 1000);
        printf("humidity:%.3f\n",     (float)record.sData.humidity / 1000);
        printf("HCHO:%.3f\n",         (float)record.sData.HCHO / 1000);
        printf("CO2:%.3f\n",          (float)record.sData.CO2 / 1000);
        printf("cellVoltage:%.3f\n",  (float)record.sData.cellVoltage / 1000);
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

static int deal_phone_online(clientClassTypedef *cc)
{
    int res = 0;
    int len_t = 4;
    unsigned char ackBuf[PROC_CONTENT_LEN] = {0};
    unsigned short ackLen = 0;
    unsigned char *p = &cc->recvBuf.contentBuf[1];
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

static int deal_phone_query(clientClassTypedef *cc)
{
    int res = 0;
    int len_t = 4;
    unsigned char ackBuf[PROC_CONTENT_LEN] = {0};
    unsigned short ackLen = 0;
    unsigned char *p = &cc->recvBuf.contentBuf[1];
    unsigned char dev_id[17] = {0};
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
                recordSentCnt = (PROC_CONTENT_LEN - 6 - 2) / sizeof(SampleDataTypedef);
                printf("recordSentCnt:%d\n",recordSentCnt);
                memcpy(ackBuf + len_t,&recordSentCnt,2);
                len_t += 2;
                memcpy(ackBuf + len_t,&cc->pRecord.recordBuf[cc->pRecord.pUpload],sizeof(SampleDataTypedef) * recordSentCnt);
                len_t += sizeof(SampleDataTypedef) * recordSentCnt;
                cc->pRecord.pkgTotal--;
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

static int deal_phone_follow_ack(clientClassTypedef *cc)
{
    int res = 0;
    int len_t = 4;
    unsigned char ackBuf[PROC_CONTENT_LEN] = {0};
    unsigned short ackLen = 0;
    int recordSentCnt = 0;

    if(cc->pRecord.pkgTotal == 1)
    {
        recordSentCnt = cc->pRecord.lastPkgCnt;
        printf("cc->pRecord.pkgTotal:%d\n",cc->pRecord.pkgTotal);
        ackBuf[len_t++] = 0x04;
        memcpy(ackBuf + len_t,&recordSentCnt,2);
        len_t += 2;
        memcpy(ackBuf + len_t,&cc->pRecord.recordBuf[cc->pRecord.pUpload],sizeof(SampleDataTypedef) * recordSentCnt);
        len_t += sizeof(SampleDataTypedef) * recordSentCnt;
        ackLen = api_procComm.makeAproc(ackBuf,len_t - 4);
        send(cc->socket,ackBuf,ackLen,0);

        free((void *)cc->pRecord.recordBuf);
        memset((void *)&cc->pRecord,0,sizeof(foundRecordsCacheTypedef));

    }
    else if(cc->pRecord.pkgTotal > 1)
    {
        printf("cc->pRecord.pkgTotal:%d\n",cc->pRecord.pkgTotal);

        ackBuf[len_t++] = 0x14;
        recordSentCnt = (PROC_CONTENT_LEN - 6 - 2) / sizeof(SampleDataTypedef);
        memcpy(ackBuf + len_t,&recordSentCnt,2);
        len_t += 2;
        memcpy(ackBuf + len_t,&cc->pRecord.recordBuf[cc->pRecord.pUpload],sizeof(SampleDataTypedef) * recordSentCnt);
        len_t += sizeof(SampleDataTypedef) * recordSentCnt;
        cc->pRecord.pkgTotal--;
        cc->pRecord.pUpload += recordSentCnt;

        ackLen = api_procComm.makeAproc(ackBuf,len_t - 4);
        send(cc->socket,ackBuf,ackLen,0);
    }
    else if(cc->pRecord.pkgTotal == 0)
    {

    }

    return res;
}

//==================================================================
//协议处理脚本注册
//==================================================================

static ProcScriptTypedef procScript[] = {
    {CTRL_DEV_ONLINE,   deal_dev_online},
    {CTRL_DEV_HEART,    deal_dev_heart},
    {CTRL_DEV_PUBLISH,  deal_dev_publish},
    {CTRL_PHONE_ONLINE, deal_phone_online},
    {CTRL_PHONE_QUERY,  deal_phone_query},
    {CTRL_PHONE_FOLLOW_ACK,deal_phone_follow_ack},
};

//==================================================================
//单个client实例化
//==================================================================
void app_process(int SocketNbr)
{
    int res,res_getProc,i;
    clientClassTypedef cc;
    api_cc.init(&cc,SocketNbr);
    
    while(1)
    {
        //从socket接受数据
        res = api_procComm.readSocket(cc.socket,&cc.recvBuf); 
        
        //超时或错误，退出线程，退出tcp链接
        if(res <= 0)
        {
            printf("app_process will exit\n");
            if((unsigned char *)cc.pRecord.recordBuf != 0)
            {
                printf("free pRecord cause pRecord=%x\n",cc.pRecord.recordBuf);
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