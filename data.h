#ifndef _DATA_H
#define _DATA_H

#define TRUE    1
#define FALSE   0

typedef struct{
    unsigned int    timeTick;   //时间戳
    int             tempture;   //温度
    int             humidity;   //湿度
    int             HCHO;       //甲醛
    int             CO2;        //二氧化碳
    int             cellVoltage;//锂电池电压
}SampleDataTypedef;//整数，除以1000保留到小数点后三位

typedef struct{
    unsigned char dev_id[17];
    SampleDataTypedef sData;
}recordTypedef;

#endif