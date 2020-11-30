#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>

//开启调试信息输出
#define ENABLE_DBG      1       

#if (ENABLE_DBG == 1)
#define DBG_PRT(fmt...)   \
    do {\
        printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
        printf(fmt);\
    }while(0)
#else
#define DBG_PRT(fmt...)
#endif

//返回值定义
#define TRUE    1
#define FALSE   0


#endif