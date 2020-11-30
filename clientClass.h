#ifndef _CLIENTCLASS_H
#define _CLIENTCLASS_H

#include "common.h"
#include "proc.h"
#include "sqlite3.h"
#include "data.h"

typedef struct{
    SampleDataTypedef *recordBuf;   //记录数组
    unsigned short recordCnt;       //总记录数
    unsigned short lastPkgCnt;      //最后一包中包含的记录数
    unsigned short pkgTotal;        //总包数
    unsigned short pUpload;         //上传记录指针
}foundRecordsCacheTypedef;

typedef struct{
    int s_scb_nbr;                  //套接字控制块序号
    int socket;                     //套接字
    sqlite3 *db;                    //数据库指针
    recvChannelfTypedef recvBuf;    //接受缓存

    foundRecordsCacheTypedef pRecord;//查询历史
}clientClassTypedef;                //客户端类

typedef struct{
    int (*init)(clientClassTypedef *cc,int s_scb_nbr);
    int (*deinit)(clientClassTypedef *cc);
}api_clientClassTypedef;            //客户端类对外api接口

extern api_clientClassTypedef api_cc;

#endif