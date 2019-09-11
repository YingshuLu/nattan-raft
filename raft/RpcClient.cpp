#include <string>
#include "RpcClient.h"
#include "Net.h"
#include "proto/http/HttpParser.h"
#include "net/SocketHelper.h"

using namespace nattan;

RpcClient::RpcClient(const Address& addr, Chan<MessagePtr>* pchan, const long ms):
    fMsgs(8), fRpcChan(pchan), kAddr(addr), kTimeout(ms) {} 

const std::string RpcClient::getAddress() {
    return kAddr.address + std::to_string(kAddr.port);
}

void RpcClient::run() {
    int res = -1;
    MessagePtr pmsg = nullptr;

    while(true) {
        pmsg = fMsgs.recv();

        do {
            if (!fSock.valid()) {
                SOCKET sockfd = SocketHelper::TcpConnectTill(kAddr, kTimeout);
                fSock.attach(sockfd);
                if (!fSock.valid()) {
                    fSock.close();
                    break;
                }
            }

            // send request
            if (!SendRequest(fSock.getSocket(), *pmsg)) {
                fSock.close();
                pmsg->clear();
                break;
            }
            
            //recv response
            if(!RecvResponse(fSock.getSocket(), *pmsg)) {
                fSock.close();
                pmsg->clear();
                break;
            }
        } while(0);

        Json::Value& root = pmsg->getRoot();
        root["address"] = getAddress();
        notify(pmsg); 
    }
}

bool RpcClient::replicate(MessagePtr& pmsg) {
    long firstTime = UUID::getTimeStamp();
    if(!fMsgs.sendTill(pmsg, kTimeout)) {
        bTimeout = true;
        return false;
    }
    return true;
}
/*
    long currentTime = UUID::getTimeStamp();
    long leftMs = kTimeout - (currentTime - firstTime);
    if (leftMs <= 0) {
        bTimeout = true;
        return false;
    }
    
    //timeout
    if(!pmsg->waitTill(leftMs)) {
        bTimeout = true;  
        return false;
    }

    // check type
    char type = pmsg->getType();
    bool validType = true;
    if (!(RAFT_MSG_VOTE_RESP == type 
            || RAFT_MSG_APPD_RESP == type
            || RAFT_MSG_DATA_RESP == type)) {
        validType = false;
    }

    //pmsg not change
    if (!validType) return false;
    return true;
}
*/

void RpcClient::notify(MessagePtr& pmsg) {
    if (!bTimeout) {
        fRpcChan->send(pmsg);
    }
    bTimeout = false;
}

