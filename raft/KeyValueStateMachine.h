#ifndef RAFT_KEY_VALUE_STATE_MACHINE_H
#define RAFT_KEY_VALUE_STATE_MACHINE_H

#include <assert.h>
#include "StateMachine.h"

class KeyValueStateMachine {
public:
    KeyValueStateMachine();

public:
    Json::Value apply(const Json::Value& cmd);
    
    bool dumpToSnapshot(const char* snapshot);

    bool loadFromSnapshot(const char* snapshot);
    
    ~KeyValueStateMachine() {}

privagte:
    RocksDB kDB;
    static const char* kDBPath;
};

#endif
