#pragma once

#include <netinet/in.h>

#include "server.h"

class MsgServer : public Server {

public:
    MsgServer(int, bool);
    ~MsgServer();

protected:
    void create();
    void close_socket();

private:
    int port_;
};
