
#ifndef RAFT_CONFIG_FILE_H
#define RAFT_CONFIG_FILE_H

#include <stdlib.h>
#include <string>
#include "core/ConfigParser.h"
#include "thread/ThreadMutex.h"

class ConfigFile {

public:
    ConfigFile(const std::string& filename) : kFilename(filename) {}
    bool load() {
        if(nattan::File::Isexisted(kFilename)) return false;
        LockGuard guard(kMutex);
        kConfig.load(kFilename);
        return true;
    }

    std::string getString(const std::string& sec, const std::string& key) {
        return kConfig.getValueByKey(sec, key);
    }

    int getInt(const std::string& sec, const std::string& key) {
        std::string value = getString(sec, key);
        return atoi(value.data());
    }

    int getInt(const std::string& sec, const std::string& key) {
        std::string value = getString(sec, key);
        return atol(value.data());
    }

    std::string getString(const std::string& sec, const std::string& key, std::string& defaultValue) {
        std::string value = getString(sec, key);
        if (value.empty()) value = defaultValue;
        return value;
    }

    int getInt(const std::string& sec, const std::string& key, int defaultValue) {
        std::string value = getString(sec, key);
        if (value.empty()) return defaultValue;
        return atoi(value.data());
    }

    long getLong(const std::string& sec, const std::string& key, long defaultValue) {
        std::string value = getString(sec, key);
        if (value.empty()) return defaultValue;
        return atol(value.data());
    }

private:
    const std::string kFilename;
    nattan::ThreadMutex kMutex;
    nattan::ConfigParser kConfig;
};


#endif
