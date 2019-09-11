#ifndef RAFT_NET_H
#define RAFT_NET_H

#include "net/Socket.h"
#include "Message.h"

bool RecvRequest(const nattan::SOCKET sockfd, Message& msg);
bool RecvResponse(const nattan::SOCKET sockfd, Message& msg);
bool SendRequest(const nattan::SOCKET sockfd, Message& msg);
bool SendResponse(const nattan::SOCKET sockfd, Message& msg);

#endif
