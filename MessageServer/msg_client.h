#pragma once

#include <netinet/in.h>

class MsgClient : public Client {

public:
	MsgClient(int port, string server, bool debug);
	~MsgClient();

}