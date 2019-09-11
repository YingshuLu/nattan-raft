#include "net/Socket.h"
#include "DataServer.h"
#include "Net.h"

using namespace nattan;

class DataHandler: public Runnable {
public:
    DataHandler(const SOCKET sockfd, Chan<MessagePtr>* chanptr):
        fSock(sockfd), chanptr(pchan) {}
    
    void run() {
        MessagePtr pmsg(new Message());
        while(true) {
            if (!RecvRequest(fSock.getSocket(), *pmsg)) {
                fSock.close();
                break;
            }

        }
    }

private:
    Socket fSock;
    Chan<MessagePtr>* pchan;
};

