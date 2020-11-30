#ifndef _CLIENTCLASS_H
#define _CLIENTCLASS_H

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
    int SocketNbr;
    int socket;
    sqlite3 *db;
    recvChannelfTypedef recvBuf;

    foundRecordsCacheTypedef pRecord;
}clientClassTypedef;

typedef struct{
    int (*init)(clientClassTypedef *cc,int SocketNbr);
    int (*deinit)(clientClassTypedef *cc);
}api_clientClassTypedef;

extern api_clientClassTypedef api_cc;

#endif