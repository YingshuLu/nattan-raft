#include <algorithm>
#include "stdlib.h"
#include "Node.h"

using namespace nattan;

static const char* kConfigPath = "./raft.ini"
static volatile TaskThread* gRoutine = nullptr;

static void SubmitTask(std::shared_ptr<Runnable>& runner) {
    gRoutine->submit(runner);
}

static PeerPtr NewPeer(const Address& addr) {
    PeerPtr p = PeerPtr(new Peer());
    p->nextIndex = 0;
    p->matchedIndex = 0;
    p->pcli = RpcClientPtr(new RpcClient(addr));
    return p;
}


//===Node server===

NodeServer::NodeServer():
    msgChan(16),
    rpcChan(16),
    kShmData(kNodePath, sizeof(Node), true),
    conf(kConfigPath) {
        pinfo = (Node*)kShmData.open();
        heartbeatTimeout = conf.getLong("timeout", "heartbeat");
        selectionTimeout = conf.getLong("timeout", "selection");
        randSelectionTimeout = selectionTimeout + rand() % (200);
}  

void NodeServer::start() {
    if (gRoutine != nullptr) return;
    gRoutine = new TaskThread(10 * 1000);
    gRoutine->start();
    std::shared_ptr<Runnable> nodeRunner(new NodeServer());
    gRoutine->submit(nodeRunner);
    gRoutine->join();
    delete gRoutine;
}

void NodeServer::run() {
    Address mgmtAddr("0.0.0.0", 8001);
    std::shared_ptr<Runnable> mgmtServerRunner(new RpcServer(mgmtAddr, &msgChan));
    SubmitTask(mgmtServerRunner);

    Address dataAddr("0.0.0.0", 8000);
    std::shared_ptr<Runnable> dataServerRunner(new RpcServer(dataAddr, &msgChan));
    SubmitTask(dataServerRunner);
    loop();
}

bool NodeServer::addPeer(const Address& addr) {
    PeerPtr p = NewPeer(addr);
    peers[p->pcli->getAddress()] = p;
    return true;
}

bool NodeServer::replicate(MessagePtr& pmsg) {
    int type = pmsg->getType();

    int nums = peers.size();
    for (auto it = peers.begin(); it != peers.end(); it ++) {
        PeerPtr& peer = it->second;
        MessagePtr pmsg0(new Message());
        pmsg0->setRoot(pmsg->getRoot());
        if (type == RAFT_MSG_APPD_REQ) {
            
        }
        peer->replicate(pmsg0);
    }

    int cnt = 0;
    pmsg = nullptr;
    while(cnt < nums) {
        rpcChan.recvTill(pmsg, 300);
        if (valid(pmsg)) {
                    
        }
    }

    

}

void NodeServer::loop() {
    while(true) {
        switch(status) {
            case FOLLOWER:
                Follower();
                break;
            case CANDIDATE:
                Candidate();
                break;
            case LEADER:
                Leader();
                break;
            default:
        }
        ApplyCommittedLog();
    }
}


void NodeServer::Leader() {
    
    

}

void NodeServer::Candidate() {
    pinfo->currentTerm += 1;
    //vote for self
    memset(pinfo->voteFor, pinfo->id, sizeof(pinfo->id));

    int voteCount = 1;
    MessagePtr pmsg = nullptr;
    randSelectionTimeout = selectionTimeout + rand() % (200);

    VoteRequest request;
    request.setTerm(pinfo->currentTerm);
    request.setCandidateId(pinfo->id);
    request.setLastLogIndex(fLog.lastIndex());
    request.setLastLogTerm(fLog.lastTerm());
    MessagePtr votemsg(new Message());
    votemsg->setJsonMessage(request);
    peers.replicateAll(votemsg);

    if (!msgChan.waitTill(randSelectionTimeout, pmsg)) {
        peers.kill();
    }

    int type = pmsg->getType();
    switch (type) {
        case RAFT_MSG_APPD_REQ:
            CandidateHandleAppendRequest(pmsg);
            break;
        case RAFT_MSG_VOTE_REQ:
            CandidateHandleVoteRequest(pmsg);
            break;
        case RAFT_MSG_VOTE_REQ:
            CandidateHandleVoteResponse(pmsg);
            break;       
        default:
    }    
}

void NodeServer::CandidateHandleAppendRequest(MessagePtr& pmsg) {
    Json::Value& json = pmsg->getJsonMessage();
    AppendRequest request;
    request.setRoot(json);
    int term = request.getTerm();
    std::string leaderId = request.getLeaderId();
    if (term > pinfo->currentTerm) {
        pinfo->currentTerm = term;
        memcpy(pinfo->voteFor, leaderId.data(), leaderId.length());
        status = FOLLOWER;
        FollowerHandleAppendRequest(pmsg);
        return;
    }
    Result result;
    result.setResult(false);
    result.setTerm(pinfo->currentTerm);
    pmsg->setJsonMessage(result);
    pmsg->notify();
}

void NodeServer::CandidateHandleVoteRequest(MessagePtr& pmsg) {
    Json::Value& json = pmsg->getJsonMessage();
    VoteRequest request;
    request.setRoot(json);
    int term = request.getTerm();
    std::string candidateId = request.getCandidateId();
    bool success = false;
    if (term > pinfo->currentTerm) {
        memcpy(pinfo->voteFor, candidateId.data(), candidateId.length());
        pinfo->currentTerm = term;
        status = FOLLOWER;
        success = true;
    }

    Result result;
    result.setResult(success);
    result.setTerm();
    pmsg->setJsonMessage(result);
    pmsg->notify();
}

void NodeServer::CandidateHandleVoteResponse(MessagePtr& pmsg) {
    Json::Value& json = pmsg->getJsonMessage();
    Result result;
    result.setRoot(json);
    if (result.getResult()) {
        status = LEADER;
    } 
}

void NodeServer::Follower() {
   MessagePtr pmsg = nullptr;
   bool success = msgChan.recvTill(pmsg, randSelectionTimeout);
   if (!success) {
      status = CANDIDATE;     
      return;
   }
   Json::Value& json = pmsg->getJsonMessage();
   int type = json["type"];
   switch (type) {
        case RAFT_MSG_APPD_REQ: 
            FollowerHandleAppendRequest(pmsg);
            break;
        case RAFT_MSG_VOTE_REQ:
            FollowerHandlerVoteRequest(pmsg);
            break;
        default:
            //ignore
   }
}

void NodeServer::FollowerHandleAppendRequest(MessagePtr& pmsg) {
    Json::Value& json = pmsg->getJsonMessage();
    AppendRequest request;
    request.setRoot(json["data"]);
    Result result;
    bool success = true;

    do {
        if (request.getTerm() < pinfo->currentTerm) {
            success = false;
            break;
        } else {
            // update term
            pinfo->currentTerm = request.getTerm();
            std::string leaderId = request.getLeaderId();
            memcpy(pinfo->voteFor, leaderId.data(), leaderId.length());
        } 

        int prevLogIndex = request.getPrevLogIndex();
        int prevLogTerm = request.getPrevLogTerm();

        Entry e;
        // check if prevLog matched
        if (!(success = fLog.retrieve(prevLogIndex, e)) 
                || !(success = (prevLogTerm == e.getTerm()))) {
            success = false;
            break;
        }

        Json::Value& entries = request.getRoot()["entries"];
        int index = preLogIndex;
        Entry t;
        for (int i = 0; i < entries.size(); i++) {
            e.setRoot(entries[i]);
            if (fLog.retrieve(++index, t)) {
                if (t.getTerm() != e.getTerm()) {
                    fLog.setLastIndex(index);
                    fLog.setLastTerm(e.getTerm());
                    fLog.put(index, e);
                }
                /* else 
                 *      ignore
                 */
            } else {
                fLog.append(e);
            }
        }

        int leaderCommit = request.getLeaderCommit();
        pinfo->commitIndex = std::min(leaderCommit, fLog.lastIndex());
    } while(0);

    result.setResult(suceess);
    result.setTerm(pinfo->currentTerm);
    pmsg->setJsonMessage(result);
    pmsg->nofify();
}

void NodeServer::FollowerHandleVoteRequest(MessagePtr& pmsg) {
    Json::Value& json = pmsg->getJsonMessage();
    VoteRequest request;
    request.setRoot(json);
    bool success = false;

    do {
        int term = request.getTerm();
        std::string candidateId = request.getCandidateId();
        int lastLogIndex = request.getLastLogIndex();
        int lastLogTerm  = request.getLastLogTerm();

        if (term < pinfo->currentTerm) break;
        if (term > pinfo->currentTerm) {
            pinfo->currentTerm = term;
            memcpy(pinfo->voteFor, candidateId.data(), candidateId.length());
        }

        std::string voteFor = pinfo->voteFor;
        if (voteFor.empty() || voteFor == candidateId) {
            if (lastLogTerm >=  fLog.lastTerm() && lastLogIndex >= fLog.lastIndex()) {
                success = true;
            }
        }
    } while(0);

    result.setResult(success);
    result.setTerm(pinfo->currentTerm);
    pmsg->setJsonMessage(result);
    pmsg->nofify();
}


void NodeServer::ApplyCommittedLog() {
    Entry e;
    for (int index = pinfo->lastApplied; index <= pinfo->commitIndex; index ++) {
        fLog.retrieve(index, e);
        fStateMachine.apply(e);
    }    
    pinfo->lastApplied = pinfo->commitIndex;
}


