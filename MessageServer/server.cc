#include <map>
#include <iostream>
#include "server.h"
#include "util.h"

Server::Server() {
    // setup variables
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    messageMap_ = new messageMap();
}

Server::~Server() {
    delete buf_;
    //delete messageMap_
    std::map::iterator it;
    for (it = messageMap_->begin(); it != messageMap_->end(); ++it)
    {
        delete it->second; // delete the vector inside the map.
    }
    delete messageMap_;

}

void
Server::run() {
    // create and run the server
    create();
    serve();
}

void
Server::create() {
}

void
Server::close_socket() {
}

void
Server::serve() {
    // setup client
    int client;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

      // accept clients
    while ((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0) {

        handle(client);
    }
    close_socket();
}

void
Server::handle(int client) {
    // loop to handle all requests
    while (1) {
        // get a request
        string request = get_request(client);
        // break if client is done or an error occurred
        if (request.empty())
            break;
        // send response
        bool success = parse_request(client,request);
        // break if an error occurred
        if (not success)
            break;
    }
    close(client);
}

string
Server::get_request(int client) {
    string request = "";
    // read until we get a newline
    while (request.find("\n") == string::npos) {
        int nread = recv(client,buf_,1024,0);
        if (nread < 0) {
            if (errno == EINTR)
                // the socket call was interrupted -- try again
                continue;
            else
                // an error occurred, so break out
                return "";
        } else if (nread == 0) {
            // the socket is closed
            return "";
        }
        // be sure to use append in case we have binary data
        request.append(buf_,nread);
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
    return request;
}

bool
Server::parse_request(int client, string request) {
    std::vector<std::string> tokens = util::split(request, ' ');
    if(tokens.size() < 1){ //check vector length
        cout<<"Invalid Command/n";
        if(debug_) cout<<"split token < 1";
        return false;
    }
    string command = tokens.at(0);
    cout << command;
    if(command == "put"){
        return put_command(tokens);
    }else if(command == "list"){
        return list_command(tokens);
    }else if(command == "get"){
        return get_command(tokens);
    }else if(command == "reset"){
        exit(0);
    }
    return send_response(client,"error: invalid command");
}

bool Server::put_command(std::vector<std::string> tokens) {
    string response;
    if(tokens.size() < 4){ //check vector length
        if(debug_) cout<<"not enough arguments";
        return send_response("error: invalid formatted message");
    }
//check if user exists
    //if not create vector
    //add subject and message to the map
    //respond OK

    return send_response("OK\n");
}

bool Server::list_command(std::vector<std::string> tokens) {
    if(tokens.size() < 2){ //check vector length
        if(debug_) cout<<"not enough arguments";
        return send_response("error: invalid formatted message");
    }
    string response;
    //get vector assosiated with the name
    //list [number]\n
    //iterate through and add each subject with a index
    return send_response(response);
}

bool Server::get_command(std::vector<std::string> tokens) {
    if(tokens.size() < 3){ //check vector length
        if(debug_) cout<<"not enough arguments";
        return send_response("error: invalid formatted message");
    }
    string response;
    //use user and inde to find message
    //response = message [subject] [length]\n[message]
    return send_response(response);
}

bool
Server::send_response(int client, string response) {
    // prepare to send response
    const char* ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(client, ptr, nleft, 0)) < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                continue;
            } else {
                // an error occurred, so break out
                perror("write");
                return false;
            }
        } else if (nwritten == 0) {
            // the socket is closed
            return false;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return true;
}
