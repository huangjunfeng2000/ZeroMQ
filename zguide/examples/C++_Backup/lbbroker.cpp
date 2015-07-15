//
//  Least-recently used (LRU) queue device
//  Clients and workers are shown here in-process
//
// Olivier Chamoux <olivier.chamoux@fr.thalesgroup.com>

#include "zhelpers.hpp"
#include <pthread.h>
#include <queue>
#include <set>

//  Basic request-reply client using REQ socket
//
static void *
client_thread (void *arg) {
	//zmq::context_t context(1);
	zmq::context_t *context = (zmq::context_t *)(arg);
    zmq::socket_t client (*context, ZMQ_REQ);
	srand(time(0));
    s_set_id (client);			//  Makes tracing easier
    client.connect("inproc://frontend1");

    //  Send request, get reply
    s_send (client, "HELLO");
    std::string reply = s_recv (client);
    std::cout << "Client: " << reply << std::endl;
	//s_sleep(1000);
    return (NULL);
}

//  Worker using REQ socket to do LRU routing
//
static void *
worker_thread (void *arg) {
	//zmq::context_t context(1);
	zmq::context_t *context = (zmq::context_t *)(arg);
    zmq::socket_t worker (*context, ZMQ_REQ);
	srand(time(0));
    s_set_id (worker);          //  Makes tracing easier
    worker.connect("inproc://backend1");

    //  Tell backend we're ready for work
    s_send (worker, "READY");

    while (1) {
        //  Read and save all frames until we get an empty frame
        //  In this example there is only 1 but it could be more
        std::string address = s_recv (worker);
        {
            std::string empty = s_recv (worker);
            assert (empty.size() == 0);
        }

        //  Get request, send reply
        std::string request = s_recv (worker);
        std::cout << "Worker: " << request << std::endl;
		if (request == "end")
			break;
        s_sendmore (worker, address);
        s_sendmore (worker, "");
        s_send     (worker, "OK");
    }
    return (NULL);
}

int main16 (int argc, char *argv[])
{

    //  Prepare our context and sockets
    zmq::context_t context(1);
    zmq::socket_t frontend (context, ZMQ_ROUTER);
    zmq::socket_t backend (context, ZMQ_ROUTER);
    frontend.bind("inproc://frontend1");
    backend.bind("inproc://backend1");
	//s_sleep (1000);
    int client_nbr;
	for (client_nbr = 0; client_nbr < 10; client_nbr++) {
		pthread_t client;
		pthread_create (&client, NULL, client_thread, &context);
	}
    int worker_nbr;
    for (worker_nbr = 0; worker_nbr < 3; worker_nbr++) {
        pthread_t worker;
        pthread_create (&worker, NULL, worker_thread, &context);
		//s_sleep (100);
    }
	//for (client_nbr = 0; client_nbr < 10; client_nbr++) {
	//	pthread_t client;
	//	pthread_create (&client, NULL, client_thread, &context);
	//	s_sleep (100);
	//}
    //  Logic of LRU loop
    //  - Poll backend always, frontend only if 1+ worker ready
    //  - If worker replies, queue worker as ready and forward reply
    //    to client if necessary
    //  - If client requests, pop next worker and send request to it
    //
    //  A very simple queue structure with known max size
    std::queue<std::string> worker_queue;
	std::set<std::string> setWorkers;
    while (1) {
    	
        //  Initialize poll set
        zmq::pollitem_t items [] = {
            //  Always poll for worker activity on backend
            { backend,  0, ZMQ_POLLIN, 0 },
            //  Poll front-end only if we have available workers
            { frontend, 0, ZMQ_POLLIN, 0 }
        };
		//std::stringstream ss;
		//ss << worker_queue.size();
		//std::cout << "worker_queue size: " << worker_queue.size() << std::endl;
        if (worker_queue.size())
            zmq::poll (&items [0], 2, -1);
        else
            zmq::poll (&items [0], 1, -1);

        //  Handle worker activity on backend
        if (items [0].revents & ZMQ_POLLIN) {
        	
            //  Queue worker address for LRU routing
            worker_queue.push(s_recv (backend));
			setWorkers.insert(worker_queue.back());
            {
               //  Second frame is empty
               std::string empty = s_recv (backend);
               assert (empty.size() == 0);
            }

            //  Third frame is READY or else a client reply address
            std::string client_addr = s_recv (backend);

            //  If client reply, send rest back to frontend
            if (client_addr.compare("READY") != 0) {

                {
                    std::string empty = s_recv (backend);
                    assert (empty.size() == 0);
                }
                
                std::string reply = s_recv (backend);
                s_sendmore (frontend, client_addr);
                s_sendmore (frontend, "");
                s_send     (frontend, reply);
                
                if (--client_nbr == 0)
                    break;
            }
        }
        if (items [1].revents & ZMQ_POLLIN) {
        	
            //  Now get next client request, route to LRU worker
            //  Client request is [address][empty][request]
            std::string client_addr = s_recv (frontend);
            
            {
                std::string empty = s_recv (frontend);
                assert (empty.size() == 0);
            }
            
            std::string request = s_recv (frontend);

            std::string worker_addr = worker_queue.front();//worker_queue [0];
            worker_queue.pop();

			s_sendmore (backend, worker_addr);
			s_sendmore (backend, "");
			s_sendmore (backend, client_addr);
			s_sendmore (backend, "");
			s_send     (backend, request);

			std::cout << "Worker Addr: " << worker_addr << " Client Addr: " << client_addr << std::endl;

        }
    }
	//zmq::socket_t client (context, ZMQ_REQ);
	//client.connect("inproc://backend1");
	typedef std::set<std::string>::iterator iter;
	for (iter it=setWorkers.begin(); it!=setWorkers.end(); ++it)
	{
		//s_send     (client, "end");//s_sendmore (backend, "");s_send     (backend, "end");
		//s_sleep (1000);
		s_sendmore     (backend, *it);s_sendmore (backend, "");
		s_sendmore     (backend, *it);s_sendmore (backend, "");s_send     (backend, "end"); 
	}

	//s_sendmore     (backend, "end");s_sendmore (backend, "");s_send     (backend, "end"); 
	//s_sleep (1000);
	//s_sendmore     (backend, "end");s_sendmore (backend, "");s_send     (backend, "end"); 
	//s_sleep (1000);
    //s_sleep (2000);


	//zmq_close (frontend);
	//zmq_close (backend);
	//zmq_term (context);
    return 0;
}
