/*
 ============================================================================
 Name        : daoperate.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "includes.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "sys/time.h"

//*******************************************
//common define
//*******************************************
#define DATABASE_PATH       "MyDataBase.db"
#define TOTAL_TABLE_NAME    "TotalTable"

#define CMD_EXEC_SIZE       1024

#define DB_TOTAL_TABLE_PARA     "(DevID TEXT PRIMARY KEY,Tick integer,\
                        Tempture integer,Humidity integer,HCHO integer,CO2 integer,CellVoltage integer)"

#define DB_CHILD_TABLE_PARA     "(Tick integer PRIMARY KEY,\
                        Tempture integer,Humidity integer,HCHO integer,CO2 integer,CellVoltage integer)"

#define INSERT_TOTAL_TABLE_PARA "(DevID,Tick,Tempture,Humidity,HCHO,CO2,CellVoltage)"
#define INSERT_CHILD_TABLE_PARA "(Tick,Tempture,Humidity,HCHO,CO2,CellVoltage)"

//*******************************************
//static define
//*******************************************

typedef struct{
    int (*insert)   (sqlite3 * db,recordTypedef *record,int type);
    int (*find)     (sqlite3 * db,recordFindTypedef *recordfind,int type);
    int (*change)   (sqlite3 * db,recordTypedef *record,int type);
}DB_BspTypedef;         //数据库增删查改底层

typedef struct{
    int cnt;
    int ticks[300];
    int ids[300];
}S_FindTickTypedef;     //按Tick查询时使用 

//*******************************************
//static global
//*******************************************
static char g_path[PATH_SIZE] = {0};
static char cmd_exec[CMD_EXEC_SIZE] = {0};
static S_FindTickTypedef s_findtick;

static void clear_cmd_exec()
{
    cmd_exec[0] = 0;
}

//多进程共同操作数据库函数
static int loop_sqlite3_exec(
  sqlite3* db,                                  
  const char *sql,                           
  int (*callback)(void*,int,char**,char**),  
  void *para,                                    
  char **errmsg                              
)
{
    int res;
    int cnt = 20;
    struct timeval tval1,tval2;

    gettimeofday(&tval1,NULL);

    do
    {
        res = sqlite3_exec(db,sql,callback,para,errmsg);
        if(res == SQLITE_BUSY)
        {
            DB_PRT("database is busy,left try time=%d\n",cnt);
            usleep(500000);
            continue;
        }
        break;
    }while(cnt--);

    gettimeofday(&tval2,NULL);

    printf("exec time past %d us\n",(tval1.tv_sec==tval2.tv_sec ? (tval2.tv_usec - tval1.tv_usec):((tval2.tv_sec - tval1.tv_sec - 1)*1000000 + 1000000-tval1.tv_usec + tval2.tv_usec)));

    return res;
}

//*******************************************
//底层接口->DB_BspTypedef中的函数
//*******************************************
static int find_total_table_callback(void *para,int colNum,char **colValue ,char **colName)
{
    ((recordFindTypedef *)para)->findcnt++;
    ((recordFindTypedef *)para)->sData[0].timeTick = atoi(colValue[1]);
    ((recordFindTypedef *)para)->sData[0].tempture = atoi(colValue[2]);
    ((recordFindTypedef *)para)->sData[0].humidity = atoi(colValue[3]);
    ((recordFindTypedef *)para)->sData[0].HCHO = atoi(colValue[4]);
    ((recordFindTypedef *)para)->sData[0].CO2 = atoi(colValue[5]);
    ((recordFindTypedef *)para)->sData[0].cellVoltage = atoi(colValue[6]);
    return 0;
}

static int find_child_table_callback(void *para,int colNum,char **colValue ,char **colName)
{
    int index = ((recordFindTypedef *)para)->findcnt++ ;
    if(index < MAX_RECORD_CNT)
    {
        ((recordFindTypedef *)para)->sData[index].timeTick = atoi(colValue[0]);
        ((recordFindTypedef *)para)->sData[index].tempture = atoi(colValue[1]);
        ((recordFindTypedef *)para)->sData[index].humidity = atoi(colValue[2]);
        ((recordFindTypedef *)para)->sData[index].HCHO = atoi(colValue[3]);
        ((recordFindTypedef *)para)->sData[index].CO2 = atoi(colValue[4]);
        ((recordFindTypedef *)para)->sData[index].cellVoltage = atoi(colValue[5]);
    }
    
    return 0;
}


static int bsp_insert   (sqlite3 * db,recordTypedef *record,int type)
{
    int res = SQLITE_OK;
    unsigned char cmd_exec[CMD_EXEC_SIZE] = {0};
    char *errMsg = 0;
    char *addr = cmd_exec;

    if(type == TOTAL_TABLE)
    {
        snprintf(addr,CMD_EXEC_SIZE - strlen(cmd_exec),"REPLACE INTO \"%s\" %s ",TOTAL_TABLE_NAME,INSERT_TOTAL_TABLE_PARA);
        addr += strlen(cmd_exec);
        snprintf(addr,CMD_EXEC_SIZE - strlen(cmd_exec),"VALUES(\"%s\",%d,%d,%d,%d,%d,%d)",
                                        record->dev_id,
                                        record->sData.timeTick,
                                        record->sData.tempture,
                                        record->sData.humidity,
                                        record->sData.HCHO,
                                        record->sData.CO2,
                                        record->sData.cellVoltage
                                        );
    }
    else if(type == CHILD_TABLE)
    {
        snprintf(addr,CMD_EXEC_SIZE - strlen(cmd_exec),"REPLACE INTO \"dev_%s\" %s ",record->dev_id,INSERT_CHILD_TABLE_PARA);
        addr += strlen(cmd_exec);
        snprintf(addr,CMD_EXEC_SIZE - strlen(cmd_exec),"VALUES(%d,%d,%d,%d,%d,%d)",
                                        record->sData.timeTick,
                                        record->sData.tempture,
                                        record->sData.humidity,
                                        record->sData.HCHO,
                                        record->sData.CO2,
                                        record->sData.cellVoltage
                                        );
    }

    DB_PRT("%s\n",cmd_exec);
    res = loop_sqlite3_exec(db,cmd_exec,NULL,NULL,&errMsg);
    if(res != SQLITE_OK)
    {
        DB_PRT("insert fail with %d -%s\n",res,*errMsg);
    }
    sqlite3_free(errMsg);
    return res;
}

static int bsp_find     (sqlite3 * db,recordFindTypedef *paramfind,int type)
{
    int res = SQLITE_OK;
    char *errMsg = 0;
    unsigned char cmd_exec[CMD_EXEC_SIZE] = {0};
    int multiFlag = 0;
    int i;
    char *addr = cmd_exec;

    if(type == TOTAL_TABLE)
    {
        snprintf(addr,CMD_EXEC_SIZE - strlen(cmd_exec),
            "SELECT * FROM %s WHERE ",
            TOTAL_TABLE_NAME);
        addr += strlen(cmd_exec);
        paramfind->findcnt = 0;
        snprintf(addr,CMD_EXEC_SIZE - strlen(cmd_exec),"DevID=\'%s\'",paramfind->dev_id);

        DB_PRT("%s\n",cmd_exec);

        res = loop_sqlite3_exec(db,cmd_exec,find_total_table_callback,(void *)paramfind,&errMsg);
        if(res != SQLITE_OK)
        {
            DB_PRT("find fail with %d -%s\n",res,*errMsg);
        }
    }
    else if(type == CHILD_TABLE)
    {
        snprintf(addr,CMD_EXEC_SIZE - strlen(cmd_exec),
            "SELECT * FROM \'dev_%s\' WHERE ",
            paramfind->dev_id);
        addr += strlen(cmd_exec);
        paramfind->findcnt = 0;
        snprintf(addr,CMD_EXEC_SIZE - strlen(cmd_exec),"Tick>=%d AND Tick<=%d",paramfind->starttick,paramfind->EndTick);

        DB_PRT("%s\n",cmd_exec);

        res = loop_sqlite3_exec(db,cmd_exec,find_child_table_callback,(void *)paramfind,&errMsg);
        if(res != SQLITE_OK)
        {
            DB_PRT("find fail with %d -%s\n",res,*errMsg);
        }
    }

    sqlite3_free(errMsg);

    return res;
}

static int bsp_change(sqlite3 * db,recordTypedef *record,int type)
{
    int res = SQLITE_OK;
    unsigned char cmd_exec[CMD_EXEC_SIZE] = {0};
    char *errMsg = 0;
    char *addr = cmd_exec;

    if(type == UPDATE_DATA)
    {
        snprintf(addr,CMD_EXEC_SIZE - strlen(cmd_exec),"UPDATE \"%s\"",TOTAL_TABLE_NAME);
        addr += strlen(cmd_exec);
        snprintf(addr,CMD_EXEC_SIZE - strlen(cmd_exec),"SET Tempture=%d,Humidity=%d,HCHO=%d,CO2=%d,CellVoltage=%d WHERE DevID=\'%s\'",
                                        record->sData.tempture,
                                        record->sData.humidity,
                                        record->sData.HCHO,
                                        record->sData.CO2,
                                        record->sData.cellVoltage,
                                        record->dev_id
                                        );
    }
    else if(type == UPDATE_ONLINE_TICK)
    {
        snprintf(addr,CMD_EXEC_SIZE - strlen(cmd_exec),"UPDATE \"%s\"",TOTAL_TABLE_NAME);
        addr += strlen(cmd_exec);
        snprintf(addr,CMD_EXEC_SIZE - strlen(cmd_exec),"SET Tick=%d WHERE DevID=\'%s\'",
                                        record->sData.timeTick,
                                        record->dev_id
                                        );
    }

    DB_PRT("%s\n",cmd_exec);
    res = loop_sqlite3_exec(db,cmd_exec,NULL,NULL,&errMsg);
    if(res != SQLITE_OK)
    {
        DB_PRT("change fail with %d -%s\n",res,*errMsg);
    }
    sqlite3_free(errMsg);
    return res;
}




//*******************************************
//函数注册
//*******************************************

static DB_BspTypedef bsp_DB = {
    .insert     =   bsp_insert,
    .find       =   bsp_find,
    .change     =   bsp_change
};

//*******************************************
//上层接口->DataBaseTypedef中的函数
//*******************************************

static int api_init(sqlite3 **db)
{
    int res = SQLITE_OK;
    unsigned char cmd_exec[CMD_EXEC_SIZE] = {0};
    res = sqlite3_open(DATABASE_PATH,db);
    printf("db addr: %x\n",*db);
    if(res != SQLITE_OK)
    {
        DB_PRT("open fail with %d-%s\n",res,sqlite3_errmsg(*db));
    }

    //创建表
    snprintf(cmd_exec,CMD_EXEC_SIZE,"create table if not exists \'%s\' %s",TOTAL_TABLE_NAME,DB_TOTAL_TABLE_PARA);
    DB_PRT("%s\n",cmd_exec);
    loop_sqlite3_exec(*db,cmd_exec,NULL,NULL,NULL);

    return res;
}

static int api_deinit(sqlite3 *db)
{
    int res = SQLITE_OK;
    res = sqlite3_close(db);
    if(res != SQLITE_OK)
    {
        DB_PRT("close fail with %d-%s\n",res,sqlite3_errmsg(db));
    }

    return res;
}

static int api_dev_online(recordTypedef *record,sqlite3 *db)
{
    int res = SQLITE_OK;
    recordFindTypedef recordFind;
    memset((void *)&recordFind,0,sizeof(recordFindTypedef));
    memcpy(recordFind.dev_id,record->dev_id,16);

    //创建表
    snprintf(cmd_exec,CMD_EXEC_SIZE,"create table if not exists \'dev_%s\' %s",record->dev_id,DB_CHILD_TABLE_PARA);
    DB_PRT("%s\n",cmd_exec);
    res = loop_sqlite3_exec(db,cmd_exec,NULL,NULL,NULL);
    if(res != SQLITE_OK)
    {
        DB_PRT("api_dev_online create table fail with %d\n",res);
    }
    else
    {
        res = bsp_DB.find(db,&recordFind,TOTAL_TABLE);
        if(res != SQLITE_OK)
        {
            DB_PRT("api_dev_online find fail with %d\n",res);
        }
        else
        {
            //未找到该ID记录，插入
            if(recordFind.findcnt == 0)
            {
                res = bsp_DB.insert(db,record,TOTAL_TABLE);
                if(res != SQLITE_OK)
                {
                    DB_PRT("api_dev_online insert fail with %d\n",res);
                }
            }
            //已存在该ID的记录，则更新上线时间
            else
            {
                res = bsp_DB.change(db,record,UPDATE_ONLINE_TICK);
                if(res != SQLITE_OK)
                {
                    DB_PRT("api_dev_online change fail with %d\n",res);
                }
            }
        }
        
    }

    return res;
}

//res:1 dev exist   0:dev not exist
static int api_phone_online(recordTypedef *record,sqlite3 *db)
{
    int res = SQLITE_OK;
    recordFindTypedef recordFind;
    memset((void *)&recordFind,0,sizeof(recordFindTypedef));
    memcpy(recordFind.dev_id,record->dev_id,16);

    res = bsp_find(db,&recordFind,TOTAL_TABLE);
    if(res == SQLITE_OK)
    {
        if(recordFind.findcnt != 0)
        {
            res = TRUE;
            memcpy(&record->sData,&recordFind.sData[0],sizeof(SampleDataTypedef));
        }
        else
        {
            res = FALSE;
        }
    }
    else
    {
        res = FALSE;
    }

    return res;
}

static int api_devPublish(recordTypedef *record,sqlite3 *db)
{
    int res = SQLITE_OK;

    res = bsp_DB.insert(db,record,CHILD_TABLE);
    if(res == SQLITE_OK)
    {
        res = bsp_DB.change(db,record,UPDATE_DATA);
        res = TRUE;
    }
    else
    {
        res = FALSE;
    }

    return res;
}

static int api_phoneQuery(unsigned char *dev_id,int startTick,int endTick,foundRecordsCacheTypedef *pRecord,sqlite3 *db)
{
    int res = SQLITE_OK;
    int lenOfOnePgk = PROC_CONTENT_LEN - 8;
    recordFindTypedef recordFind;
    memset((void *)&recordFind,0,sizeof(recordFindTypedef));
    memcpy(recordFind.dev_id,dev_id,16);
    recordFind.starttick = startTick;
    recordFind.EndTick   = endTick;


    //先看该设备存在于否
    res = bsp_find(db,&recordFind,TOTAL_TABLE);
    if(res == SQLITE_OK)
    {
        if(recordFind.findcnt != 0)
        {
            //若存在，再开始按时间段查询
            res = bsp_find(db,&recordFind,CHILD_TABLE);
            if(res == SQLITE_OK)
            {
                DB_PRT("find records %d\n",recordFind.findcnt);
                //找到时间段内的数据
                if(recordFind.findcnt != 0)
                {
                    if(recordFind.findcnt > MAX_RECORD_CNT)
                    {
                        DB_PRT("only get 24 * 31 records\n");
                        recordFind.findcnt = MAX_RECORD_CNT;
                    }

                    pRecord->recordCnt = recordFind.findcnt;
                    pRecord->recordBuf = (SampleDataTypedef *)malloc(sizeof(SampleDataTypedef) * pRecord->recordCnt);
                    memset(pRecord->recordBuf,0,sizeof(SampleDataTypedef) * pRecord->recordCnt);
                    pRecord->pkgTotal = (recordFind.findcnt * sizeof(SampleDataTypedef) + lenOfOnePgk - 1) / lenOfOnePgk;
                    pRecord->lastPkgCnt = recordFind.findcnt % (PROC_CONTENT_LEN / sizeof(SampleDataTypedef));
                    memcpy((void *)pRecord->recordBuf,(void *)recordFind.sData,sizeof(SampleDataTypedef) * pRecord->recordCnt);
                    res = TRUE;
                }
                //无该时间段内的数据
                else
                {
                    res = FALSE;
                }
            }
            else
            {
                res = FALSE;
            }
        }
        else
        {
            res = FALSE;
        }
    }
    else
    {
        res = FALSE;
    }
    
    return res;
}

//*******************************************
//函数注册
//*******************************************
api_DataBaseTypedef api_DB = {
    .init       =   api_init,
    .deinit     =   api_deinit,
    .devOnline  =   api_dev_online,
    .phoneOnline=   api_phone_online,
    .devPublish =   api_devPublish,
    .phoneQuery =   api_phoneQuery
};

//为测试创建数据库
void createDbForTest()
{
    int i;
    sqlite3 *db;
    api_DB.init(&db);

    recordTypedef record;

    //step1.simulate dev online
    memcpy(record.dev_id,"1234567812345678",17);
    record.sData.timeTick = 1000000;
    record.sData.tempture = 0;
    record.sData.humidity = 0;
    record.sData.HCHO     = 0;
    record.sData.CO2      = 0;
    record.sData.cellVoltage = 0;

    api_DB.devOnline(&record,db);      

    //step2.simulate dev publish 2000 records
    for(i = 0;i < 2000; i++)
    {
        record.sData.timeTick = 1000000 + i;
        record.sData.tempture = i;
        record.sData.humidity = i;
        record.sData.HCHO     = i;
        record.sData.CO2      = i;
        record.sData.cellVoltage = i;

        api_DB.devPublish(&record,db);
    }

    api_DB.deinit(db);
}