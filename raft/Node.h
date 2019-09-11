#ifndef RAFT_NODE_H
#define RAFT_NODE_H

#include <assert.h>
#include <memory>
#include <map>
#include "sync/Chan.h"
#include "core/SharedMemoryData.h"
#include "thread/Runnable.h"
#include "task/TaskThread.h"
#include "Message.h"
#include "RpcClient.h"
#include "KeyValueStateMachine.h"

typedef std::shared_ptr<RpcClient> RpcClientPtr;
enum NODE_STATUS {
    FOLLOWER,
    CANDIDATE,
    LEADER
};

struct Node {
    NODE_STATUS status;
    int currentTerm;
    int commitIndex;
    int lastApplied;
    char votedFor[256];
    char id[256];
};

struct Peer {
    int nextIndex;
    int matchedIndex;
    RpcClientPtr pcli;
};

typedef std::shared_ptr<Peer> PeerPtr;

class NodeServer: public nattan::Runnable {
public:
    NodeServer();
    static void start();

protected:
    void run();

private:
    bool addPeer(const Address& addr);

    void Leader();
    void Candiadte();
    void Follower();
    
    void CandidateHandleAppendRequest(MessagePtr& pmsg);
    void CandidateHandleVoteRequest(MessagePtr& pmsg);
    void CandidateHandleVoteResponse(MessagePtr& pmsg);

    void FollowerHandleAppendRequest(MessagePtr& pmsg);
    void FollowerHandleVoteRequest(MessagePtr& pmsg);
    
    void ApplyCommittedLog();


public:
    nattan::Chan<MessagePtr> msgChan;
    nattan::Chan<MessagePtr> rpcChan;
    std::list<PeerPtr> peers;
    Node* pInfo = nullptr;
    LogEntry fLog;
    nattan::SharedMemoryData kShmData;
    KeyValueStateMachine fStateMachine;
    ConfigFile conf;
    std::map<std::string, PeerPtr> peers;

    long heartbeatTimeout;
    long selectionTimeout;
    long randSelectionTimeout;

private:
    static const char* kNodePath;
};

const char* Node::kNodePath = "raft_node_info.dat";

#endif
