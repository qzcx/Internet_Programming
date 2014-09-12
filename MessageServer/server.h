#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <set>
#include <vector>

using namespace std;

class Server {
public:
    Server();
    ~Server();

    void run();
    
protected:
    virtual void create();
    virtual void close_socket();
    void serve();
    void handle(int);
    string get_request(int);
    bool parse_request(int, string);
    bool send_response(int, string);

    bool put_command(std::vector<std::string> tokens);
    bool list_command(std::vector<std::string> tokens);
    bool get_command(std::vector<std::string> tokens);


    int server_;
    int buflen_;
    char* buf_;

    struct message{
        string subject;
        string message;
    };

    typedef map<string,vector<message>> messageMap;
    messageMap *messageMap_;
    bool debug_;



};
