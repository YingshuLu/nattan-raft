#ifndef RAFT_STATEMACHINE_H
#define RAFT_STATEMACHINE_H

#include <string>
#include "core/File.h"
#include "deps/jsoncpp/include/json/json.h"

class StateMachine {

public:
    virtual Json::Value apply(const Json::Value& cmd) = 0;
    virtual bool dumpAsSnapshot(File& file) = 0;
    virtual bool loadFromSnapshot(File& file) = 0;
    virtual ~StateMachine(){}

};

#endif
