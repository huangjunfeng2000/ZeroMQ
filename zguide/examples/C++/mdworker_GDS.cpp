//
//  Majordomo Protocol worker example
//  Uses the mdwrk API to hide all MDP aspects
//
//  Lets us 'build mdworker' and 'build all'
//
//     Andreas Hoelzlwimmer <andreas.hoelzlwimmer@fh-hagenberg.at>
//
#include "mdwrkapi_gds.hpp"

int main (int argc, char *argv [])
{

    int verbose = true;//(argc > 1 && strcmp (argv [1], "-v") == 0);
	char *strProject = (argc > 2) ? argv[2] : "c:\\Test\\1.uni";
    GDSmdwrk session ("tcp://localhost:5555", strProject, verbose);

    zmsg *reply = 0;
    while (1) {
        zmsg *request = session.recv (reply);
        if (request == 0) {
            break;              //  Worker was interrupted
        }
        reply = request;        //  Echo is complex... :-)
    }
    return 0;
}
