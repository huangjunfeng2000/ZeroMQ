//
//  Round-trip demonstrator
//
//  While this example runs in a single process, that is just to make
//  it easier to start and stop the example. Each thread has its own
//  context and conceptually acts as a separate process.
//
//    Andreas Hoelzlwimmer <andreas.hoelzlwimmer@fh-hagenberg.at>
//
#include "zmsg.hpp"
#include <pthread.h>

static void *
client_task (void *args)
{
    zmq::context_t context (1);
    zmq::socket_t client (context, ZMQ_DEALER);
    client.setsockopt (ZMQ_IDENTITY, "C", 1);
	int s1;
	size_t r1 = sizeof(int);

	int jsendsize = 100000;
	client.getsockopt(ZMQ_SNDHWM, &s1, &r1);
	client.setsockopt (ZMQ_SNDHWM, &jsendsize, sizeof(jsendsize));
	client.getsockopt(ZMQ_SNDHWM, &s1, &r1);

	int jrecesize = 100000;
	client.setsockopt(ZMQ_RCVHWM, &jrecesize, sizeof(jrecesize));
 
	client.getsockopt(ZMQ_SNDBUF, &s1, &r1);
	int jsendsize2 = 100000 * 200;
	client.setsockopt(ZMQ_SNDBUF, &jsendsize2, sizeof(jsendsize2));
	client.getsockopt(ZMQ_RCVBUF, &s1, &r1);

	int jrecesize2 = 100000 * 200;
	client.setsockopt(ZMQ_RCVBUF, &jrecesize2, sizeof(jrecesize2));

	client.connect ("tcp://localhost:5555");

    std::cout << "Setting up test..." << std::endl;
    s_sleep (1000);

    int requests;
    int64_t start;

    std::cout << "Synchronous round-trip test..." << std::endl;
    start = s_clock ();
    for (requests = 0; requests < 10000; requests++) {
		zmsg msg ("HELLO");
		msg.send (client);
		msg.recv (client);
    }
    std::cout << (1000 * 10000) / (int) (s_clock () - start) << " calls/second" << std::endl;

    std::cout << "Asynchronous round-trip test..." << std::endl;
    start = s_clock ();
	char str[20];
    for (requests = 0; requests < 100000; requests++) {
		sprintf(str, "H_%d", requests+1);
		zmsg msg (str);
		msg.send (client);
	}
	for (requests = 0; requests < 100000; requests++) {
		zmsg msg (client); 
		//if ((requests + 1) % 1000 == 0)
		//	std::cout << "Asynchronous receive requests..." << msg.body() << " " << requests << std::endl;

    }
    std::cout << (1000 * 100000) / (int) (s_clock () - start) << " calls/second" << std::endl;

    return 0;
}

static void *
worker_task (void *args)
{
    zmq::context_t context (1);
    zmq::socket_t worker (context, ZMQ_DEALER);
    worker.setsockopt (ZMQ_IDENTITY, "W", 1);

	//int jsendsize2 = 100000 * 100;
	//int s3 = zmq_setsockopt (worker, ZMQ_SNDBUF, &jsendsize2, sizeof(jsendsize2));
	//int64_t jrecesize2 = 100000 * 100;
	//int s4 = zmq_setsockopt (worker, ZMQ_RCVBUF, &jrecesize2, sizeof(jrecesize2));

    worker.connect ("tcp://localhost:5556");

    while (1) {
        zmsg msg (worker);
        msg.send (worker);
    }
    return 0;
}

static void *
broker_task (void *args)
{
    //  Prepare our context and sockets
    zmq::context_t context (1);
    zmq::socket_t frontend (context, ZMQ_ROUTER);
    zmq::socket_t backend  (context, ZMQ_ROUTER);


    frontend.bind ("tcp://*:5555");
    backend.bind  ("tcp://*:5556");

    //  Initialize poll set
    zmq::pollitem_t items [] = {
        { frontend, 0, ZMQ_POLLIN, 0 },
        { backend,  0, ZMQ_POLLIN, 0 }
    };
    while (1) {
        zmq::poll (items, 2, -1);
        if (items [0].revents & ZMQ_POLLIN) {
            zmsg msg (frontend);
            msg.pop_front ();
            msg.push_front ((char *)"W");
            msg.send (backend);
        }
        if (items [1].revents & ZMQ_POLLIN) {
            zmsg msg (backend);
            msg.pop_front ();
            msg.push_front ((char *)"C");
            msg.send (frontend);
        }
    }
    return 0;
}

int main45 ()
{
    s_version_assert (2, 1);

    pthread_t client;
    pthread_create (&client, NULL, client_task, NULL);
    pthread_t worker;
    pthread_create (&worker, NULL, worker_task, NULL);
    pthread_t broker;
    pthread_create (&broker, NULL, broker_task, NULL);
    pthread_join (client, NULL);
    return 0;
}
