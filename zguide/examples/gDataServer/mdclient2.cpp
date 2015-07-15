//
//  Majordomo Protocol client example - asynchronous
//  Uses the mdcli API to hide all MDP aspects
//
//  Lets us 'build mdclient' and 'build all'
//
//     Andreas Hoelzlwimmer <andreas.hoelzlwimmer@fh-hagenberg.at>
//
#include "mdcliapi2.hpp"

int main50 (int argc, char *argv [])
{
    int verbose = (argc > 1 && strcmp (argv [1], "-v") == 0);
    mdcli2 session ("tcp://localhost:5555", verbose);

	int iTotal = 100000;
    int count;
    for (count = 0; count < iTotal; count++) {
        zmsg * request = new zmsg("Hello world");
        session.send ("echo", request);
    }
    for (count = 0; count < iTotal; count++) {
        zmsg *reply = session.recv ();
        if (reply) {
            delete reply;
        } else {
            break;              //  Interrupted by Ctrl-C
        }
    }
    std::cout << count << " replies received" << std::endl;
    return 0;
}
