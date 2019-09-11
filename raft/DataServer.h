#ifndef RAFT_DATA_SERVER_H
#define RAFT_DATA_SERVER_H

#include "thread/Runnable.h"
#include "net/Address.h"
#include "sync/Chan.h"
#include "Message.h"

class DataServer: public nattan::Runnable {
public:
    DataServer(const nattan::Address& addr, nattan::Chan<MessagePtr>* pchan);
    void run();

private:
    const nattan::Address kAddr;
    nattan::Chan<MessagePtr>* fchanptr;

};

#endif
