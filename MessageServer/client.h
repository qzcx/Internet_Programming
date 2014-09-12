#pragma once

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "util.h"

using namespace std;

class Client {
public:
    Client();
    ~Client();

    void run();

protected:
    bool parse_request(string request);
    bool send_command(std::vector<std::string> tokens);
    bool list_command(std::vector<std::string> tokens);
    bool read_command(std::vector<std::string> tokens);
    virtual void create();
    virtual void close_socket();
    void echo();
    bool send_request(string);
    bool get_response();

    int server_;
    int buflen_;
    char* buf_;

    bool debug_;
};
