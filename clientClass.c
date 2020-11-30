#include "clientClass.h"
#include "database.h"
#include "server.h"

static int api_cc_init(clientClassTypedef *cc,int SocketNbr)
{
    memset(cc,0,sizeof(clientClassTypedef));
    cc->SocketNbr = SocketNbr;
    cc->socket    = s_scb[SocketNbr].socket_id;
    api_DB.init(&cc->db);
    api_procComm.procInit(&cc->recvBuf);

    return 1;
}

static int api_cc_deinit(clientClassTypedef *cc)
{
    api_DB.deinit(cc->db);

    return 1;
}

api_clientClassTypedef api_cc = {
    .init = api_cc_init,
    .deinit = api_cc_deinit
};