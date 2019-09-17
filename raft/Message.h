#ifndef RAFT_MESSAGE_H
#define RAFT_MESSAGE_H

#include <vector>
#include <string>
#include <memory>
#include "deps/jsoncpp/include/json/json.h"
#include "sync/Chan.h"
#include "Command.h"

/**
*    node VOTE request: 
*    {
*        "data" : 
*        {
*            "candidate_id" : 12,
*            "last_log_index" : 124,
*            "last_log_term" : 1,
*            "term" : 1
*        },
*        "type" : 1
*    }
*    
*    node APPEND request: 
*    {
*        "data" : 
*        {
*            "entries" : 
*            [
*                {
*                    "commands" : 
*                    [
*                        {
*                            "key" : "test1",
*                                "method" : 0,
*                                "value" : "val1"
*                        },
*                        {
*                            "key" : "test2",
*                            "method" : 1
*                        },
*                        {
*                            "key" : "test3",
*                            "method" : 2
*                        }
*                    ]
*                }
*            ],
*            "leader_commit" : 11,
*            "leader_id" : 14,
*            "prev_log_index" : 243,
*            "prev_log_term" : 12,
*            "term" : 1
*        },
*        "type" : 0
*    }
*    
*    RESULT for node request: 
*    {
*        "data" : 
*        {
*            "success" : true,
*            "term" : 12
*        },
*        "type" : 128
*    }
*    
*    client request command: 
*    {
*        "data" : 
*        {
*            "commands" : 
*            [
*                {
*                    "key" : "test1",
*                        "method" : 0,
*                        "value" : "val1"
*                },
*                {
*                    "key" : "test2",
*                    "method" : 1
*                },
*                {
*                    "key" : "test3",
*                    "method" : 2
*                }
*            ]
*        },
*        "type" : 2
*    }
*    
*    response for client command: 
*    {
*        "data" : 
*        {
*            "results" : 
*            [
*                null,
*                "value2",
*                "value3"
*            ],
*            "success" : true
*        },
*        "type" : 130
*    }
**/

class MsgFuture {
public:
    MsgFuture(): chan(1) {}
    void wait(){
        chan.recv();
    }

    bool waitTill(time_t timeout_ms) {
        return chan.recvTill(result, timeout_ms);
    }

    void notify() {
        int signal = 1;
        chan.send(signal);
    }

    void notifyWithResult(const int res) {
        int signal = res;
        chan.send(signal);
    }

    int fetchResult() {
        return result;
    }

    virtual ~MsgFuture() {}

private:
    nattan::Chan<int> chan;
    int result = 0;
};

class JsonMessage {
public:
    std::string dumpToString() {
        Json::StreamWriterBuilder jsonWriterBuilder; 
        if (kJsonStringDebug) jsonWriterBuilder["indentation"] = "";
        std::unique_ptr<Json::StreamWriter> writer(jsonWriterBuilder.newStreamWriter());                                         
        std::ostringstream os;                                                                                                   
        writer->write(root, &os);                                                                                     
        return os.str();
    }

    bool loadFromString(const char* s, const size_t len) {
        Json::CharReaderBuilder builder;
        builder["collectComments"] = false;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        JSONCPP_STRING errs;
        if (!reader->parse(s, s + len, &root, &errs)) return false;
        return true;
    }

    void setRoot(Json::Value& value) {
        root = value;
    }

    Json::Value& getRoot() {
        return root;
    }

    int type() {
        return kType;
    }
    
    void clear() {
        root.clear();
    }

protected:
    JsonMessage() {}
    virtual ~JsonMessage() {}

    void setType(const int& t) {
        kType = t;
    }

    Json::Value root;

private:
    int kType = -1;
    static bool kJsonStringDebug;

};

#ifndef RAFT_JSON_DEBUG
bool JsonMessage::kJsonStringDebug = false;
#else 
bool JsonMessage::kJsonStringDebug = true;
#endif

#define RAFT_MSG_REQUEST    0
#define RAFT_MSG_RESPONSE   (1 << 7)

#define RAFT_MSG_APPD_REQ   (0 | RAFT_MSG_REQUEST)
#define RAFT_MSG_APPD_RESP  (0 | RAFT_MSG_RESPONSE)

#define RAFT_MSG_VOTE_REQ   (1 | RAFT_MSG_REQUEST)
#define RAFT_MSG_VOTE_RESP  RAFT_MSG_APPD_RESP

#define RAFT_MSG_DATA_REQ   (2 | RAFT_MSG_REQUEST)
#define RAFT_MSG_DATA_RESP  (2 | RAFT_MSG_RESPONSE)


class Command: public JsonMessage {

public:

    void putCommand(const std::string& key, const std::string& value) {
        setMethod(METHOD_PUT);
        setKey(key);
        setValue(value);
    }

    void getCommand(const std::string& key) {
        setMethod(METHOD_GET);
        setKey(key);
    }

    void delCommand(const std::string& key) {
        setMethod(METHOD_DEL);
        setKey(key);
    }
    
    void setMethod(const int& cmd) {
        root["method"] = cmd;
    }

    int getMethod() {
        return root["method"].asInt();
    }

    void setKey(const std::string& key) {
        root["key"] = key;
    }

    std::string getKey() {
        return root["key"].asString();
    }

    void setValue(const std::string& value) {
        root["value"] = value;
    }

    std::string getValue() {
        return root["value"].asString();
    }
};

class DataRequest: public JsonMessage {
public:
    DataRequest() {
        setType(RAFT_MSG_DATA_REQ);
    }

    void appendCommand(Command& cmd) {
        root["commands"].append(cmd.getRoot());
    }

    Json::Value& getCommands() {
        return root["commands"];
    }

    void setTerm(const int& term) {
        root["term"] = term;
    }

    int getTerm() {
        return root["term"].asInt();
    }
   
};

class DataResponse: public JsonMessage {
public:
    DataResponse() {
        setType(RAFT_MSG_DATA_RESP);
    }

    void setResult(const bool s) {
        root["success"] = s;
    }

    bool getResult() {
        return root["success"].asBool();
    }

    void appendString(const std::string& v) {
        root["results"].append(v);
    }

    void appendNull() {
        root["results"].append(Json::Value::null);
    }

    void appendInt(const int& v) {
        root["results"].append(v);
    }

    void getValues(std::vector<std::string>& res) {
        Json::Value values = root["results"];
        for (int i = 0; i < values.size(); i++) {
            res.push_back(values[i].asString());
        }
    }

};

typedef DataRequest Entry;

class AppendRequest: public JsonMessage {
public:
    AppendRequest() {
        setType(RAFT_MSG_APPD_REQ);
        root["entries"] = Json::Value(Json::arrayValue);
    }

    void setTerm(const int& term) {
        root["term"] = term;
    }

    int getTerm() {
        return root["term"].asInt();
    }

    void setLeaderId(const std::string& id) {
        root["leader_id"] = id;
    }

    std::string getLeaderId() {
        return root["leader_id"].asString();
    }

    void setPrevLogIndex(const int& plx) {
        root["prev_log_index"] =  plx;
    }

    int getPrevLogIndex() {
        return root["prev_log_index"].asInt();
    }

    void setPrevLogTerm(const int& term) {
        root["prev_log_term"] =  term;
    }

    int getPrevLogTerm() {
        return root["prev_log_term"].asInt();
    }

    void appendEntry(Entry& entry) {
        root["entries"].append(entry.getRoot());
    }

    Json::Value getEntries() {
        return root["entries"];
    }

    void setLeaderCommit(const int index) {
        root["leader_commit"] = index;
    }

    int getLeaderCommint() {
        return root["leader_commit"].asInt();
    }
};

class VoteRequest : public JsonMessage {
public:
    VoteRequest() {
        setType(RAFT_MSG_VOTE_REQ);
    }

    void setTerm(const int& term) {
        root["term"] = term;
    }

    int getTerm() {
        return root["term"].asInt();
    }

    void setCandidateId(const std::string& id) {
        root["candidate_id"] = id;
    }

    std::string getCandidateId() {
        return root["candidate_id"].asString();
    }

    void setLastLogIndex(const int& index) {
        root["last_log_index"] = index;
    } 

    int getLastLogIndex() {
        return root["last_log_index"].asInt();
    }

    void setLastLogTerm(const int& term) {
        root["last_log_term"] = term;
    }

    int getLastLogTerm() {
        return root["last_log_term"].asInt();
    }
};

class Result: public JsonMessage {
public:
    Result() {
        setType(RAFT_MSG_APPD_RESP);
    }

    void setTerm(const int& term) {
        root["term"] = term;
    }

    int getTerm() {
        return root["term"].asInt();
    }

    void setResult(const bool& s) {
         root["success"] = s;
    }

    bool getResult() {
         return root["success"].asBool();
    }
};

class Message: public JsonMessage, public MsgFuture {
public:
    int getType() {
        return root["type"].asInt();
    }

    bool needDiscard() {
        if (root.isMember("discard")) {
            return root["discard"].asBool();
        }
        return false;
    }

    void setJsonMessage(JsonMessage& msg) {
        root["type"] = msg.type();
        root["data"] = msg.getRoot();
        root["discard"] = false;
    }

    Json::Value& getJsonMessage() {
        return root["data"]; 
    }

};

typedef std::shared_ptr<Message> MessagePtr;

#endif
