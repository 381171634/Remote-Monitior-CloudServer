# Remote-Monitior-CloudServer
1.物联网远程监测系统的云服务器代码，部署在拥有公网IP，能建立Tcp server的Linux云主机上。  
2.确保linux已经安装sqlite，ubuntu下使用apt-get install sqlite 安装。  
3.2000行代码，涵盖服务器建立、客户端并发、维护、协议解析、数据库操作。  
4.实现物联网设备上线、数据上报、数据库维护、手机APP拉取数据等业务。  
5.嵌入式物联网设备、手机APP程序见后续代码仓库。  
  
源码结构及概要如下  
├── app.c             //应用业务层  
├── app.h               
├── clientClass.c     //客户端类  
├── clientClass.h  
├── common.h          //通用定义  
├── database.c        //数据库操作  
├── database.h  
├── data.h            //数据定义  
├── includes.h  
├── main.c            //程序入口  
├── Makefile          //makefile  
├── proc.c            //协议处理  
├── proc.h  
├── README.md  
├── server.c          //服务器  
├── server.h  
├── sqlite3ext.h  
├── sqlite3.h  
└── sqlite3lib        //sqlite链接库  
　　├── libsqlite3.so -> libsqlite3.so.0.8.6  
　　├── libsqlite3.so.0 -> libsqlite3.so.0.8.6  
　　└── libsqlite3.so.0.8.6  

