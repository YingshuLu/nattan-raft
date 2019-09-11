#include "KeyValueStateMachine.h"
#include "RocksDB.h"
#include "Message.h"

const char* KeyValueStateMachine::kDBPath = "/tmp/rocksdb_raft_keyvalues";

KeyValueStateMachine::KeyValueStateMachine() {
    assert(kDB.open(kDBPath));
} 

Json::Value KeyValueStateMachine::apply(const Json::Value& cmd) {
    const Json::Value& commands = cmd["commands"];
    
    DataResponse response;
    
    bool success = true;

    for (int i = 0; i < commands.size(); i++) {
        int type = commands[i]["type"];
        std::string key = commands[i]["key"];

        bool success = true;
        switch(type) {
            case METHOD_GET: {
                std::string value;
                success = kDB.get(key, value);
                if (success) response.append(value);
                else {
                    response.appendNull();
                }
                break;
            }

            case METHOD_PUT: {
                std::string value = commands[i]["value"];
                success = kDB.put(key, value);
                response.appendNull();
                break;
            }

            case METHOD_DEL: {
                success = kDB.del(key);
                response.appendNull();
                break;
            }
        }
    }
    response.setResult(success);
    response.getRoot();
}
