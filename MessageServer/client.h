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
    bool parse_response(string response);
    bool send_command(std::vector<std::string> tokens);
    bool list_command(std::vector<std::string> tokens);
    bool read_command(std::vector<std::string> tokens);


    string get_message(int length);

    string read_message(string);

    void response_to_list(string response);
    void response_to_read(string response);
    virtual void create();
    virtual void close_socket();
    void message();
    bool send_request(string);
    string get_response();

    int server_;
    int buflen_;
    char* buf_;
    string cache_;

    bool debug_;
};
