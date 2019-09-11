#include "Message.h"
#include <iostream>
#include "RocksDB.h"
#include "LogEntry.h"

int main() {
    
    std::cout << "node vote request: " << std::endl;
    Message msg;

    VoteRequest vote_req;
    vote_req.setTerm(1);
    vote_req.setCandidateId("12");
    vote_req.setLastLogIndex(124);
    vote_req.setLastLogTerm(1);

    msg.setJsonMessage(vote_req);

    std::cout << msg.dumpToString() << std::endl;

    VoteRequest vote;
    vote.setRoot(msg.getJsonMessage());
/*
    std::cout << "term: " <<  vote.getTerm() << std::endl;
    std::cout << "candidateId: " << vote.getCandidateId() << std::endl;
    std::cout << "lastLogIndex: " << vote.getLastLogIndex() << std::endl;
    std::cout << "lastLogTerm: " << vote.getLastLogTerm() << std::endl;
*/

    std::cout << "node append request: " << std::endl;
    Command cmd1;
    cmd1.putCommand("test1", "val1");

    Command cmd2;
    cmd2.getCommand("test2");

    Command cmd3;
    cmd3.delCommand("test3");
    
    Entry entry;
    entry.appendCommand(cmd1);
    entry.appendCommand(cmd2);
    entry.appendCommand(cmd3);

    AppendRequest app_req;

    app_req.setTerm(1);
    app_req.setLeaderId("14");
    app_req.setPrevLogIndex(243);
    app_req.setPrevLogTerm(12);
    app_req.appendEntry(entry);
    app_req.setLeaderCommit(11);

    msg.setJsonMessage(app_req);
    std::cout << msg.dumpToString() << std::endl;

    std::cout << "result for node request: " << std::endl;
    Result result;
    result.setResult(true);
    result.setTerm(12);
    msg.setJsonMessage(result);
    std::cout << msg.dumpToString() << std::endl;

    std::cout << "client request command: " << std::endl;
    //client msg
    DataRequest& request = entry;
    msg.setJsonMessage(request);
    std::cout << msg.dumpToString() << std::endl;

    std::cout << "response for client command: " << std::endl;
    // response for client
    DataResponse resp;
    resp.setResult(true);
    resp.appendNull();
    resp.appendString("value2");
    resp.appendString("value3");

    msg.setJsonMessage(resp);
    std::cout << msg.dumpToString() << std::endl;

    // test rocks db
    LogEntry log;
    entry.setTerm(12);
    log.append(entry);
    log.append(entry);
    log.append(entry);
    log.append(entry);
    std::cout << "entry count: " << log.lastIndex() << std::endl;
    Entry pv;
    for (int i = 1; i <= log.lastIndex(); i++) {
        std::cout << "============" << std::endl;
        log.retrieve(i, pv);
        std::cout << " rocks json: " << pv.dumpToString() << std::endl;
    }
    return 0;
}
