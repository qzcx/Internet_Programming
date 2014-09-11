#pragma once

#include <netinet/in.h>

#include "client.h"

class MsgClient : public Client {

public:
	MsgClient(int port, string host, bool debug);
	~MsgClient();

protected:
    void create();
    void close_socket();

private:
    string host_;
    int port_;
    bool debug_;
};