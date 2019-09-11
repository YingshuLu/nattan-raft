#ifndef RAFT_RPC_CLIENT_H
#define RAFT_RPC_CLIENT_H

#include <memory>
#include <map>
#include "net/Address.h"
#include "thread/Runnable.h"
#include "net/Socket.h"
#include "Message.h"

class RpcClient: public nattan::Runnable {
public:
    RpcClient(const nattan::Address& a, nattan::Chan<MessagePtr>* pchan, const long ms);
    const std::string getAddress();
    bool replicate(MessagePtr& pmsg);
    void run();

private:
    void notify(MessagePtr& pmsg);

private:
    nattan::Chan<MessagePtr> fMsgs;
    nattan::Chan<MessagePtr>* fRpcChan;
    nattan::Socket fSock;
    const nattan::Address kAddr;
    const long kTimeout;
    bool bTimeout = false;
};

#endif
