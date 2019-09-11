g++ -g -o msg  RocksDB.cpp test.cpp ../../task/Task.cpp ../../thread/Thread.cpp -I. -I../../ -L../../lib -lcask -ljsoncpp \
-lpthread -I /usr/local/rocksdb/include/ -L/usr/local/rocksdb/lib64 -lrt -lrocksdb -std=c++11
