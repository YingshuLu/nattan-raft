#include "thread/Runnable"
#include "net/Socket.h"
#include "net/SocketHelper.h"
#include "net/Socket.h"
#include "core/Buffer.h"
#include "Net.h"
#include "RpcServer.h"

using namespace nattan;

class RpcHandler: public nattan::Runnable {
public:
    RpcHandler(const SOCKET sockfd, Chan<MessagePtr>* pchan): fSock(sockfd), chan(pchan) {}
    
    void run() {
        MessagePtr pmsg(new Message());
        while(true) {
            // failed to recv
            if (!RecvRequest(fSock.getSocket(), *pmsg)) {
                fSock.close();
                break;;
            }

            // send for process
            chan->send(pmsg);

            // wait for completed
            if (!pmsg->waitTill(200)) {
                fSock.close();
                break;
            } else {
                //failed to send
                if (!SendResponse(fSock.getSocket(), *pmsg)){
                    fSock.close();
                    break;
                }
            }
        }
    }

private:
    Socket fSock;
    Chan<MessagePtr>* chan;
};

void RpcServer::run() {
    SOCKET acceptfd = SocketHelper::TcpListen(kaddr);

    while(true) {
        SOCKET sockfd = SocketHelper::TcpAccept(acceptfd);
        std::shared_ptr<Runnable> runner(new RpcHandler(sockfd, fchanptr));
        TaskThread::CurrentTaskThread()->submit(runner);
    }
}

RpcServer::RpcServer(const nattan::Address& addr, Chan<MessagePtr>* pchan):
    kAddr(addr), fchanptr(pchan) {}

