//client
#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include "msg_client.h"

using namespace std;

int
main(int argc, char **argv)
{
    int option;
    // setup default arguments
    int port = 3000;
    string server = "localhost";
    bool debug = false;
    // process command line options using getopt()
    // see "man 3 getopt"
    while ((option = getopt(argc,argv,"s:p:d")) != -1) {
        switch (option) {
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
                server = optarg;
                break;
            case 'd':
                debug = true;
                break;
            default:
                cout << "client [-s server] [-p port] [-d]" << endl;
                exit(EXIT_FAILURE);
        }
    }

    MsgClient client = MsgClient(port, server, debug);
    client.run();
}

