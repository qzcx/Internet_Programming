#include "client.h"


Client::Client() {
    // setup variables
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
}

Client::~Client() {
}

void Client::run() {
    // connect to the server and run echo program
    create();
    echo();
}

void
Client::create() {
}

void
Client::close_socket() {
}

void
Client::echo() {
    string line;
    cout << "%";
    // loop to handle user interface
    while (getline(cin,line)) {
        // append a newline
        line += "\n";
        // send request
        bool success = parse_request(line);
        // break if an error occurred
        if (not success)
            break;
        // get a response
        success = get_response();
        // break if an error occurred
        if (not success)
            break;
        cout<<"%";
    }
    close_socket();
}

bool
Client::parse_request(string request){
    std::vector<std::string> tokens = util::split(request, " ");
    if(tokens.size() < 1){ //check vector length
        cout<<"Invalid Command/n";
        if(debug_) cout<<"split token < 1";
        return false;
    }
    string command = tokens.at(0);
    if(command == "send"){
        return send_command(tokens);
    }
}

bool
Client::send_command(std::vector<std::string> tokens){
    if(tokens.size() < 3){ //check vector length
        cout<<"send [user] [subject]/n";
        if(debug_) cout<<"not enough arguments";
        return false;
    }
    string user = tokens.at(1);
    string subject = tokens.at(2); 

    cout << "- Type your message. End with a blank line -\n";
    string line;
    string message = "";
    getline(cin,line);
    while(line != "\n"){
        message += line;
        getline(cin,line); //get next line
    }
    //create the request
    std::stringstream request;
    request << "put " << user << " " << subject << " " << message.length() << "\n" << message;
    return send_request(request.str());
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

bool
Client::get_response() {
    string response = "";
    // read until we get a newline
    while (response.find("\n") == string::npos) {
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
        response.append(buf_,nread);
    }
    // a better client would cut off anything after the newline and
    // save it in a cache
    cout << response;
    return true;
}
