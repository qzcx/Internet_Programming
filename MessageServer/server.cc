#include "server.h"

Server::Server() {
    // setup variables
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    messageMap_ = new MessageMap();
}

Server::~Server() {
    delete buf_;
    //delete messageMap_
    //MessageMap::iterator it;
    // for (it = messageMap_->begin(); it != messageMap_->end(); ++it)
    // {
    //     delete it->second; // delete the vector inside the map.
    // }
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

string
Server::get_message(int client, int length) {
    char* msgBuf = new char[length+1];
    string request = "";
    // read until we get a newline
    while (request.find("\n") == string::npos) {
        int nread = recv(client,msgBuf,1024,0);
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
        request.append(msgBuf,nread);
    }
    delete msgBuf;
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
        return put_command(client, tokens);
    }else if(command == "list"){
        return list_command(client, tokens);
    }else if(command == "get"){
        return get_command(client, tokens);
    }else if(command == "reset"){
        exit(0);
    }
    return send_response(client, "error: invalid command");
}

bool 
Server::put_command(int client, std::vector<std::string> tokens) {
    string response;
    if(tokens.size() < 4){ //check vector length
        if(debug_) cout<<"not enough arguments";
        return send_response(client, "error: invalid formatted message");
    }
    string name = tokens.at(1);
    string subject = tokens.at(2);
    int length;
    length = atoi(tokens.at(3).c_str());
    if(length < 1){
        if(debug_) cout<<"e.what()"<<endl;
        return send_response(client, "error: invalid length parameter");
    }

    string message = get_message(client, length);

    MessageMap::iterator it;

    Message* newMsg = new Message(subject, message);

    it = messageMap_->find(name);
    if(it == messageMap_->end()){ //name doesn't exist yet. add it with a new vector to the map.
        std::pair<MessageMap::iterator,bool> ret;
        vector<Message*>* messageList = new vector<Message*>();
        messageList->push_back(newMsg);
        ret = messageMap_->insert(std::pair<string,vector<Message*>* >(name,messageList));
    }else{ //append newMsg to the list
        it->second->push_back(newMsg);
    }
    //respond OK
    return send_response(client, "OK\n");
}

bool 
Server::list_command(int client,std::vector<std::string> tokens) {
    if(tokens.size() < 2){ //check vector length
        if(debug_) cout<<"not enough arguments";
        return send_response(client, "error: invalid formatted message");
    }
    string name = tokens.at(1);
    //get vector assosiated with the name
    MessageMap::iterator it;
    it = messageMap_->find(name);
    if(it == messageMap_->end()){ //name doesn't exist
        if(debug_) cout<<name <<": name doesn't exist"<<endl;
        return send_response(client, "error: " + name + ": doesn't exist");
    } 
    vector<Message*> messageVector = *(it->second);
    int length = messageVector.size();
    //list [number]\n
    std::stringstream response;
    response << "list " << length << "\n";
    //iterate through and add each subject with a index
    int i=1;
    for(std::vector<Message*>::iterator it = messageVector.begin() ; it != messageVector.end(); ++it, i++){
        response << i << " ";
        response << (*it)->subject_ << "\n";
    }
    
    return send_response(client, response.str());
}

bool 
Server::get_command(int client,std::vector<std::string> tokens) {
    if(tokens.size() < 3){ //check vector length
        if(debug_) cout<<"not enough arguments";
        return send_response(client, "error: invalid formatted message");
    }
    string name = tokens.at(1);
    int index;
        index = atoi(tokens.at(2).c_str());
    if(index < 1){
        if(debug_) cout<<"e.what()"<<endl;
        return send_response(client, "error: invalid index parameter");
    }
    //use user and index to find message
    MessageMap::iterator it;
    it = messageMap_->find(name);
    if(it == messageMap_->end()){ //name doesn't exist
        if(debug_) cout<<name <<": name doesn't exist"<<endl;
        return send_response(client, "error: " + name + ": doesn't exist");
    } 
    vector<Message*> messageVector = *(it->second);
    if(index > messageVector.size()){
        if(debug_) cout<<"index out of bounds"<<endl;
        return send_response(client, "error: index out of bounds");
    }
    Message* msg = messageVector.at(index - 1); //-1 because the index starts at 1.
    //response = message [subject] [length]\n[message]
    std::stringstream response;
    response << "message " + msg->subject_;
    response << " " << msg->text_.size();
    response << "\n" << msg->text_;
    return send_response(client, response.str());
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
