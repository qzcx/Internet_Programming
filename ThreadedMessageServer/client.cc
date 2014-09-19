#include "client.h"


Client::Client() {
    // setup variables
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    cache_ = "";
}

Client::~Client() {
    delete buf_;
}

void Client::run() {
    // connect to the server and run echo program
    create();
    message();
}

void
Client::create() {
}

void
Client::close_socket() {
}

void
Client::message() {
    string line;
    cout << "%";
    // loop to handle user interface
    while (getline(cin,line)) {
        // send request
        bool success = parse_request(line);
        // break if an error occurred
        if (not success)
            break;
        if(debug_) cout<< "cache at end=" << cache_;
        cout<<"%";
    }
    close_socket();
}

bool
Client::parse_request(string request){
    std::vector<std::string> tokens = util::split(request, ' ');
    if(tokens.size() < 1){ //check vector length
        cout<<"Invalid Command/n";
        if(debug_) cout<<"split token < 1";
        return true;
    }
    string command = tokens.at(0);
    if(debug_) cout << command;
    if(command == "send"){
        return send_command(tokens);
    }else if(command == "list"){
        return list_command(tokens);
    }else if(command == "read"){
        return read_command(tokens);
    }else if(command == "quit"){
        exit(0);
    }
    request += "\n";
    return send_request(request);
}

bool
Client::send_command(std::vector<std::string> tokens){
    if(tokens.size() < 3){ //check vector length
        cout<<"send [user] [subject]\n";
        if(debug_) cout<<"not enough arguments";
        return false;
    }
    string user = tokens.at(1);
    string subject = tokens.at(2); 

    cout << "- Type your message. End with a blank line -\n";
    string line;
    string message = "";
    getline(cin,line);
    while(!line.empty()){
        message += line + "\n";
        //if(debug_) cout << line;
        getline(cin,line); //get next line
    }
    //create the request
    std::stringstream request;
    request << "put " << user;
    request << " " << subject;
    request << " " << message.length();
    request << "\n" << message;
    if(debug_) cout << request.str();
    bool ret = send_request(request.str());
    if(debug_) cout << "return from send: " << ret <<endl;
    //send_request(message);

    string response = get_response();
    if(response != "OK")
        cout << response << endl;
    if(debug_) cout << "responsed with: " <<response << endl;
    return true;
}

bool
Client::list_command(std::vector<std::string> tokens){
    if(tokens.size() < 2){ //check vector length
        cout<<"list [user]\n";
        if(debug_) cout<<"not enough arguments";
        return false;
    }
    string user = tokens.at(1);

    //create the request
    std::stringstream request;
    request << "list " << user;
    request << "\n";
    if(debug_) cout << request.str() << endl;
    bool ret = send_request(request.str());
    if(!ret) {return ret;}
    string response = get_response();
    response_to_list(response);
    return true;
}

void
Client::response_to_list(string response){
    std::vector<std::string> responseTokens = util::split(response, ' ');
    if(responseTokens.size() != 2){ //check vector length
        if(debug_) cout<<"incorrect list response: " <<response << endl;
        return;
    }
    int number = atoi(responseTokens.at(1).c_str());
    if(number < 0){
        if(debug_) cout<<"error: invalid length parameter"<<endl;
        return;
    }
    string output;
    for(int i=0; i< number; i++){
        output = get_response();
        cout << output << "\n";
    }
}


bool
Client::read_command(std::vector<std::string> tokens){
if(tokens.size() < 3){ //check vector length
        cout<<"read [user] [index]\n";
        if(debug_) cout<<"not enough arguments";
        return false;
    }
    string user = tokens.at(1);
    string index = tokens.at(2); 

    //create the request
    std::stringstream request;
    request << "get " << user;
    request << " " << index;
    request << "\n";
    cout << request.str();
    bool ret = send_request(request.str());
    if(!ret) {return ret;}

    string response = get_response();

    response_to_read(response);
    return true;
}

void 
Client::response_to_read(string response){
    std::vector<std::string> responseTokens = util::split(response, ' ');
    if(responseTokens.size() != 3){ //check vector length
        if(debug_) cout<<"incorrect list response: " <<response << endl;
        return;
    }
    string subject = responseTokens.at(1);
    int length = atoi(responseTokens.at(2).c_str());
    if(length < 0){
        if(debug_) cout<<"error: invalid length parameter"<<endl;
        return;
    }
    string message = get_message(length);
    cout << subject << "\n";
    cout << message; //don't think I need a "\n" here
    return;
}

bool
Client::send_request(string request) {
    // prepare to send request
    const char* ptr = request.c_str();
    int nleft = request.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(server_, ptr, nleft, 0)) < 0) {
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

string
Client::get_response() {
    string response = "";
    // read until we get a newline
    while (cache_.find("\n") == string::npos) {
        int nread = recv(server_,buf_,1024,0); 
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
        cache_.append(buf_,nread);
    }
    if(debug_) cout<< "cache before read: " <<cache_ <<endl;
    response = read_message(cache_);
    
    return response;
}

string
Client::read_message(string request){
    int index = request.find("\n");
    if(debug_) cout<< "index: " << index << endl;
    if (index == string::npos){
        return "";
    }
    string message = request.substr(0,index);
    cache_ = request.substr(index+1,string::npos);
    return message;
}

string
Client::get_message(int length) {
    string message = cache_; 
    cache_ = "";
    // read until we get a newline
    while (message.size() < length) {
        //8int readNow = (length - numRead) > 1024 ? 1024 : (length - numRead);
        int nread = recv(server_,buf_,1024,0);
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
        message.append(buf_,nread);
    }
    if(message.size() > length){
        cache_ = message.substr(length+1,string::npos);
        message = message.substr(0,length);
        //if(debug_) cout<< "cache_ is now: " << cache_ << endl;
    }else{
        cache_ = "";
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
    return message;
}

