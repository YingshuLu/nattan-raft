#include <string>
#include <algorithm>
#include <rocksdb/db.h>
#include <rocksdb/slice.h>
#include <rocksdb/options.h>
#include "RocksDB.h"
#include "Command.h"

using namespace rocksdb;

#define DBHandler ((DB*)(this->fDBHandler))

bool RocksDB::open(const char* path) {
    if (path == nullptr) path = kDBPath.data();
    Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    options.create_if_missing = true;
    Status s = DB::Open(options, path, (DB**)(&(fDBHandler)));
    return s.ok();
}

bool RocksDB::put(const std::string& key, const std::string& value) {
    Status s = DBHandler->Put(WriteOptions(), key, value);
    return s.ok();
}

bool RocksDB::get(const std::string& key, std::string& value) {
    Status s = DBHandler->Get(ReadOptions(), key, &value);
    return s.ok();
}

bool RocksDB::del(const std::string& key) {
    Status s = DBHandler->Delete(WriteOptions(), key);
    return s.ok();
}

bool RocksDB::getRange(const std::string& start, const std::string& end, std::vector<std::string>& values) {
    Iterator* it = DBHandler->NewIterator(ReadOptions());
    for (it->Seek(start); it->Valid() && it->key().ToString() < end; it->Next()) {
        values.push_back(it->value().ToString());
    }
    delete it;
    return true;
}

bool RocksDB::execute(const Json::Value& value) {
    const Json::Value& commands = value["commands"];

    WriteBatch batch;
    for(int i = 0; i < commands.size(); i++) {
        int type = commands[i]["method"].asInt();
        std::string key = commands[i]["key"].asString();

        switch(type) {
            case METHOD_PUT:
                batch.Put(key, commands[i]["value"].asString());
                break;
            case METHOD_DEL:
                batch.Delete(key);
                break;
            default:
                break;    
        }
    }

    Status s = DBHandler->Write(WriteOptions(), &batch);
    return s.ok();
}

std::string RocksDB::firstKey() {
    auto it = DBHandler->NewIterator(ReadOptions());
    it->SeekToFirst();
    std::string empty;
    if (it->Valid()) {
        empty = it->key().ToString();
    }
    delete it;
    return empty;
}

std::string RocksDB::lastKey() {
    auto it = DBHandler->NewIterator(ReadOptions());
    it->SeekToLast();
    std::string empty;
    if (it->Valid()) {
        empty = it->key().ToString();
    }
    delete it;
    return empty;
}

bool RocksDB::close() {
    if (DBHandler) {
        delete DBHandler;
        fDBHandler = nullptr;
    }
}

