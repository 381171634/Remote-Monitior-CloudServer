#ifndef _APP_H
#define _APP_H
#include "proc.h"
#include "data.h"
#include "clientClass.h"
#include "common.h"

typedef enum{
    CTRL_DEV_ONLINE = 0,        //设备上线
    CTRL_DEV_HEART,             //设备心跳
    CTRL_DEV_PUBLISH,           //设备推送

    CTRL_PHONE_ONLINE,          //手机上线
    CTRL_PHONE_QUERY,           //手机查询历史记录
    CTRL_PHONE_FOLLOW_ACK = 0x14,//手机后续帧应答
    CTRL_END,
}CtrlTypedef;                   

typedef struct{
    uint8_t procID;
    int (*procDeal)(clientClassTypedef *cc);
}ProcScriptTypedef;             //协议处理脚本结构体

void app_process(int s_scb_nbr);

#endif