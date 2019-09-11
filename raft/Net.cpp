#include "sync/Timer.h"
#include "net/SocketHelper.h"
#include "core/Buffer.h"
#include "proto/http/HttpParser.h"
#include "Net.h"
#include <errno.h>

using namespace nattan;

static ssize_t Recv(const SOCKET sockfd, HttpParser& parser) {
    Socket sock(sockfd);
    sock.setTimeout(-1);
    Buffer buf;
    while(!parser.eof()) {
        int n = sock.read(buf.end(), buf.right());
        if (n <= 0) return -1;
        buf.fill(n);
        n = parser->parse(buf.data(), buf.length());
        if (n <= 0) return -1;
        buf.shrink(n);
    }
    return buf.length();
}

static ssize_t Send(const SOCKET sockfd, Buffer& buf) {
    Socket sock(sockfd);
    sock.setTimeout(-1);
    char* it = buf.data();
    while(it != buf.end()) {
        int n = sock.write(it, buf.end() - it);
        if (n <= 0) return -1;
        it += n;
    }
    return buf.length();
}

static bool Recv(const SOCKET sockfd, Message& msg) {
    HttpParser parser;
    if (Recv(sockfd, parser) <= 0) {
        return false;
    } 

    size_t len = parser.body.length();
    char* buf = new buf[len + 1];
    if (parser.body.read(buf, len) != len) return false;
    msg.loadFromString(buf, len);
    delete[] buf;
    return true;
}

bool RecvRequest(const SOCKET sockfd, Message& msg) {
    return Recv(sockfd, msg);
}

bool RecvResponse(const SOCKET sockfd, Message& msg) {
    return Recv(sockfd, msg);
}

bool SendRequest(const SOCKET sockfd, Message& msg) {
    std::string json = msg.dumpToString();
    Buffer buf;
    buf.append("POST /raft HTTP/1.1\r\n");
    buf.append("Host: raft.nattan.com\r\n");
    buf.append("Content-Type: application/json\r\n");
    buf.append("Content-Length: ");
    buf.append(std::to_string(json.length()));
    buf.append("\r\n\r\n");
    buf.append(json);
    buf.append("\r\n");
    
    int len = buf.length();
    if (Send(sockfd, buf) != len) return false;
    return true;
}

bool SendResponse(const SOCKET sockfd, Message& msg) {
    std::string json = msg.dumpToString();
    Buffer buf;
    buf.append("HTTP/1.1 200 OK\r\n");
    buf.append("Content-Type: application/json\r\n");
    buf.append("Content-Length: ");
    buf.append(std::to_string(json.length()));
    buf.append("\r\n\r\n");
    buf.append(json);
    buf.append("\r\n");

    int len = buf.length();
    if (Send(sockfd, buf) != len) return false;
    return true;
}


