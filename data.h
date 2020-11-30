#ifndef _DATA_H
#define _DATA_H

#include "common.h"

typedef struct{
    uint32_t        timeTick;   //时间戳
    int             tempture;   //温度
    int             humidity;   //湿度
    int             HCHO;       //甲醛
    int             CO2;        //二氧化碳
    int             cellVoltage;//锂电池电压
}SampleDataTypedef;//整数，除以1000保留到小数点后三位

typedef struct{
    uint8_t dev_id[17];         //设备id
    SampleDataTypedef sData;    //设备数据
}recordTypedef;

#endif