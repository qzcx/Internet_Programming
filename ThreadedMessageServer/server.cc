#include "server.h"

#define NUM_THREADS 10

#define BUFFER_SIZE 100

Server::Server() {
    // setup variables
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    messageMap_.m = new MessageMap();
    clients_.q = new std::queue<int>();
    //store_message("jon","love","I love liana");
    //store_message("jon","dog","I love chewy");
    //store_message("jon","girl","I love my wife");
}

Server::~Server() {
    delete buf_;
    delete messageMap_.m;
    delete clients_.q;

    for(int i=0;i< NUM_THREADS; i++){
        delete threads_[i];
    }
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

void* threadHandle(void * ptr ){
    ((Server *)ptr)->handle();
    return NULL;
}

void
Server::serve() {
    //setup threads
    //sem_init(messageMap_.s,0,1);
    //sem_init(clients_.s,0,1);
    //sem_init(clients_.n,0,0);
    //sem_init(clients_.e,0,BUFFER_SIZE);

    if ((messageMap_.s = sem_open("messageMap_.s", O_CREAT, 0644, 1)) == SEM_FAILED) {
        perror("semaphore initilization");
        exit(1);
    }

    if ((clients_.s= sem_open("clients_.s", O_CREAT, 0644, 1)) == SEM_FAILED) {
        perror("semaphore initilization");
        exit(1);
    }

    if ((clients_.n = sem_open("clients_.n", O_CREAT, 0644, 0)) == SEM_FAILED) {
        perror("semaphore initilization");
        exit(1);
    }

    if ((clients_.e = sem_open("clients_.e", O_CREAT, 0644, BUFFER_SIZE)) == SEM_FAILED) {
        perror("semaphore initilization");
        exit(1);
    }

    // setup client
    int client;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

    threads_ = (pthread_t**)malloc(sizeof(pthread_t*));
    for(int i=0; i<NUM_THREADS; i++){
        threads_[i] = new pthread_t();
        pthread_create(threads_[i], NULL, &threadHandle, this);
    }



      // accept clients
    while ((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0) {
        sem_wait(clients_.e); //buffer check
        sem_wait(clients_.s);
        clients_.q->push(client);
        sem_post(clients_.s);
        sem_post(clients_.n); //produce
    }

    close_socket();
}



void 
Server::handle() {
    string cache = "";
    int client;
    while(1){
        sem_wait(clients_.n); //consume
        sem_wait(clients_.s);
        client = clients_.q->front();
        clients_.q->pop();
        sem_post(clients_.s);
        sem_post(clients_.e); //buffer free
        cache = "";
        // loop to handle all requests
        string request;
        while (1) {
            // get a request
            while (cache.find("\n") == string::npos) {
                //if(debug_) cout<<"waiting\n";
                int nread = recv(client,buf_,1024,0);
                if (nread < 0) {
                    if (errno == EINTR)
                        // the socket call was interrupted -- try again
                        continue;
                    else
                        // an error occurred, so break out
                        break;
                } else if (nread == 0) {
                    // the socket is closed
                    break;
                }
                //if(debug_) cout<< "recieved: "<< buf_ << endl;
                cache.append(buf_,nread);
            }
            request = read_message(cache, &cache);
            if (request.empty()){
                break;
            }
            // send response
            bool success = parse_request(client,request, &cache);
            // break if an error occurred
            //if(debug_) cout<< "I parsed and responsed: "<< cache << endl;
            if (not success){
                break;
            }
        }
        //cout << "something bad happened";
        close(client);
    }
}


string
Server::read_message(string request, string* cache){
    int index = request.find("\n");
    if (index == string::npos){
        return "";
    }
    string message = request.substr(0,index);
    (*cache) = request.substr(index+1,string::npos);
    return message;
}

string
Server::get_message(int client, int length, string* cache) {
    string message = *cache; 
    (*cache) = "";
    // read until we get a newline
    while (message.size() < length) {
        //8int readNow = (length - numRead) > 1024 ? 1024 : (length - numRead);
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
        message.append(buf_,nread);
    }
    if(message.size() > length){
        (*cache) = message.substr(length+1,string::npos);
        message = message.substr(0,length);
        //if(debug_) cout<< "cache is now: " << cache << endl;
    }else{
        (*cache) = "";
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
    return message;
}


bool
Server::parse_request(int client, string request, string* cache) {
    std::vector<std::string> tokens = util::split(request, ' ');
    if(tokens.size() < 1){ //check vector length
        //cout<<"Invalid Command/n";
        if(debug_) cout<<"split token < 1";
        return send_response(client, "error invalid command\n");
    }
    string command = tokens.at(0);
    bool ret = false;
    if(debug_) cout << command << endl;
    if(command == "put"){
        ret = put_command(client, tokens, cache);
    }else if(command == "list"){
        ret = list_command(client, tokens);
    }else if(command == "get"){
        ret = get_command(client, tokens);
    }else if(command == "reset"){

        delete messageMap_.m;
        messageMap_.m = new MessageMap();
        ret = send_response(client, "OK\n");
    }else{
        ret = send_response(client, "error invalid command\n");
    }
    //if(debug_) cout<< "I parsed! " << ret << endl;
    return ret;
}

bool 
Server::put_command(int client, std::vector<std::string> tokens, string* cache) {
    string response;
    if(tokens.size() < 4){ //check vector length
        if(debug_) cout<<"not enough arguments";
        return send_response(client, "error invalid formatted message\n");
    }
    string name = tokens.at(1);
    string subject = tokens.at(2);
    int length;
    length = atoi(tokens.at(3).c_str());
    if(length < 1){
        return send_response(client, "error invalid length parameter\n");
    }

    string message = get_message(client, length, cache);

    store_message(name,subject,message);
    //respond OK
    return send_response(client, "OK\n");
}



bool 
Server::list_command(int client,std::vector<std::string> tokens) {
    if(tokens.size() < 2){ //check vector length
        if(debug_) cout<<"not enough arguments";
        return send_response(client, "error invalid formatted message\n");
    }
    string name = tokens.at(1);

    string response = get_subjects(name);
    //get vector assosiated with the name
    
    return send_response(client, response);
}

bool 
Server::get_command(int client,std::vector<std::string> tokens) {
    if(tokens.size() < 3){ //check vector length
        if(debug_) cout<<"not enough arguments";
        return send_response(client, "error invalid formatted message\n");
    }
    string name = tokens.at(1);
    int index;
        index = atoi(tokens.at(2).c_str());
    if(index < 1){
        if(debug_) cout<<"e.what()"<<endl;
        return send_response(client, "error invalid index parameter\n");
    }
    //use user and index to find message
    MessageMap::iterator it;
    sem_wait(messageMap_.s); //wait your turn!
    it = messageMap_.m->find(name);
    if(it == messageMap_.m->end()){ //name doesn't exist
        if(debug_) cout<<name <<": name doesn't exist"<<endl;
        sem_post(messageMap_.s); //be sure to release lock
        return send_response(client, "error " + name + ": doesn't exist\n");
    } 
    vector<Message*> messageVector = *(it->second);
    if(index > messageVector.size()){
        if(debug_) cout<<"index out of bounds"<<endl;
        sem_post(messageMap_.s); //release the lock!
        return send_response(client, "error index out of bounds\n");
    }
    Message* msg = messageVector.at(index - 1); //-1 because the index starts at 1.
    sem_post(messageMap_.s);
    //response = message [subject] [length]\n[message]
    std::stringstream response;
    response << "message " + msg->subject_;
    response << " " << msg->text_.size();
    response << "\n" << msg->text_;
    return send_response(client, response.str());
}



bool
Server::send_response(int client, string response) {
    if(debug_) cout<< "responding with :" << response;
    // prepare to send response
    const char* ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        //if(debug_) cout<< "nleft: " << nleft <<endl; 
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
    //if(debug_) cout<< "finished sending"<< endl;
    return true;
}

void
Server::store_message(string name, string subject, string message){
    //if(debug_) cout<< "storing: " << name << " " << subject << " " << message <<endl;
    if(debug_) cout<< "storing: "<<endl;
    MessageMap::iterator it;

    Message* newMsg = new Message(subject, message);
    sem_wait(messageMap_.s);
    if(debug_) cout<< "inside messageMap_.s: "<<endl;
    it = messageMap_.m->find(name);
    if(it == messageMap_.m->end()){ //name doesn't exist yet. add it with a new vector to the map.
        std::pair<MessageMap::iterator,bool> ret;
        vector<Message*>* messageList = new vector<Message*>();
        messageList->push_back(newMsg);
        ret = messageMap_.m->insert(std::pair<string,vector<Message*>* >(name,messageList));
    }else{ //append newMsg to the list
        it->second->push_back(newMsg);
    }
    sem_post(messageMap_.s);

    if(debug_) cout<< "outside messageMap_.s: "<<endl;
}

string
Server::get_subjects(string name){
    if(debug_) cout<< "looking for: "<< name << endl;
    MessageMap::iterator it;
    sem_wait(messageMap_.s);
    it = messageMap_.m->find(name);
    if(it == messageMap_.m->end()){ //name doesn't exist
        sem_post(messageMap_.s);
        return "list 0\n";
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
    sem_post(messageMap_.s);
    return response.str();
}

