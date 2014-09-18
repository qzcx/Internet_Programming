#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <exception>
#include "util.h"

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
    string read_message(string);

    string get_message(int, int);

    bool parse_request(int, string);
    bool send_response(int client, string response);

    bool put_command(int client, std::vector<std::string> tokens);
    bool list_command(int client, std::vector<std::string> tokens);
    bool get_command(int client, std::vector<std::string> tokens);

    void store_message(string name, string subject, string message);
    string get_subjects(string name);

    int server_;
    int buflen_;
    char* buf_;
    string cache_;

    struct Message{
        string subject_;
        string text_;
        Message(string subject, string text){
            subject_ = subject;
            text_ = text;
        }
    };

    typedef map<string,vector<Message*>* > MessageMap;
    MessageMap *messageMap_;
    bool debug_;



};
