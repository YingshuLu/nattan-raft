
#ifndef RAFT_RPC_SERVER_H
#define RAFT_RPC_SERVER_H

#include "net/Address.h"
#include "task/TashThread.h"
#include "sync/Chan.h"
#include "Message.h"

class RpcServer: nattan::Runnable {
public:
    RpcServer(const nattan::Address& addr, nattan::Chan<MessagePtr>* pchan);
    void run();

private:
    const nattan::Address kAddr;
    nattan::Chan<MessagePtr>* fchanptr;
};

#endif
