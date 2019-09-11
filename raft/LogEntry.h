#ifndef RAFT_LOG_ENTRY_H
#define RAFT_LOG_ENTRY_H 

#include <string>
#include <algorithm>
#include <vector>
#include <assert.h>
#include "RocksDB.h"
#include "core/SharedMemoryData.h"

struct LogIndex {
    int lastIndex;
    int lastTerm;
};

class LogEntry {
public:

    LogEntry(): kMeta(kMetaPath, sizeof(LogIndex), true) {
        assert(kDB.open(kDBPath));
        assert((fLogMeta = (LogIndex*)kMeta.open()) != nullptr);
    }

    bool append(Entry& e) {
        fLogMeta->lastTerm = e.getTerm();    
        std::string value = e.dumpToString();
        return kDB.put(nextIndexString(), value);
    }

    bool put(const int index, Entry& e) {
        std::string value = e.dumpToString();
        return kDB.put(std::to_string(index), value);
    }

    bool remove(const int index) {
        return kDB.del(std::to_string(index));
    }

    bool retrieve(const int index, Entry& e) {
        if (index > lastIndex()) return false;
        std::string value;
        bool bsucceed = kDB.get(std::to_string(index), value);
        if (bsucceed) {
            e.clear();
            e.loadFromString(value.data(), value.length());
        }
        return bsucceed;
    }

    bool retrieveRange(const int start, const int end, std::vector<Entry>& entries) {
        std::vector<std::string> res;
        bool bsucceed = kDB.getRange(std::to_string(start), std::to_string(end), res);
        if (bsucceed) {
            auto it = res.begin();
            while(it != res.end()) {
                Entry e;
                e.loadFromString(it->data(), it->length());
                entries.push_back(e);
            }
        }
        return bsucceed;
    }

    void setLastIndex(const int last) {
        fLogMeta->lastIndex = last;
    }

    void setLastTerm(const int term) {
        fLogMeta->lastTerm = term;
    }

    int lastIndex() {
        return fLogMeta->lastIndex;
    }

    int lastTerm() {
        return fLogMeta->lastTerm;
    }

    ~LogEntry() {
        LogIndex* pix = (LogIndex*)(kMeta.fHandle);
    }

private:
    std::string nextIndexString() {
        fLogMeta->lastIndex += 1;
        return std::to_string(fLogMeta->lastIndex);
    }

private:
    LogIndex* fLogMeta = nullptr;
    RocksDB kDB;
    nattan::SharedMemoryData kMeta;
    static const char* kDBPath;
    static const char* kMetaPath;

};

const char* LogEntry::kDBPath = "/tmp/rocksdb_raft_log_entry";
const char* LogEntry::kMetaPath = "raft-rocksdb-log-meta.dat";

#endif
