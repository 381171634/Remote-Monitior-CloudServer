#ifndef DBOPERATR_H_
#define DBOPERATR_H_

#include "sqlite3.h"
#include "data.h"
#include "clientClass.h"


#define PATH_SIZE           128
#define MAX_RECORD_CNT      (24*31)

//*******************************************
//common define
//*******************************************

typedef enum{
    TOTAL_TABLE = 0,
    CHILD_TABLE,
}tableTypeTypedef;

typedef enum{
    UPDATE_ONLINE_TICK = 0,
    UPDATE_DATA
}updateTypeTypedef;

typedef struct{
    int findcnt;
    uint8_t dev_id[17];
    uint32_t starttick;
    uint32_t EndTick;
    SampleDataTypedef sData[MAX_RECORD_CNT];//查找结果缓存，最大支持查询一个月
}recordFindTypedef;       //调用bsp find时的传参

typedef struct{
    int (*init)(sqlite3 **db);                              //api初始化
    int (*deinit)(sqlite3 *db);                             //api去初始化
    int (*devOnline)(recordTypedef *record,sqlite3 *db);    //设备上线
    int (*phoneOnline)(recordTypedef *record,sqlite3 *db);  //手机上线
    int (*devPublish)(recordTypedef *record,sqlite3 *db);   //设备上报
    int (*phoneQuery)(uint8_t *dev_id,uint32_t startTick,uint32_t endTick,foundRecordsCacheTypedef *pRecord,sqlite3 *db);//手机按时间段查询
}api_DataBaseTypedef;   //数据库上层接口

//*******************************************
//extern define
//*******************************************

extern api_DataBaseTypedef api_DB;
extern void createDbForTest();


#endif