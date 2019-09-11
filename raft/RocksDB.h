#ifndef RAFT_ROCKSDB_H
#define RAFT_ROCKSDB_H

#include <vector>
#include "deps/jsoncpp/include/json/json.h"

class RocksDB {
public:
    bool open(const char* path);
    bool put(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);
    bool del(const std::string& key);    
    bool getRange(const std::string& start, const std::string& end, std::vector<std::string>& values);
    bool execute(const Json::Value& value);
    std::string firstKey();
    std::string lastKey();
    bool close();

    ~RocksDB() {
        close();
    }

private:
    std::string kDBPath = "/tmp/rocksdb_raft";
    void * fDBHandler;
};

#endif
