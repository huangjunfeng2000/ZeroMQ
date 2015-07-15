/*
    Copyright (c) 2007-2014 Contributors as noted in the AUTHORS file

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../include/zmq.h"
#include "../include/zmq_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include "ServerMain.h"

int main (int argc, char *argv [])
{
	//main_rep_file_trans();
	//main_rep_file_trans_pack();
	main_rep_db_trans_pack();
	return 0;
    const char *bind_to;
    int message_count;
    size_t message_size;
    void *ctx;
    void *receive;
	void *send;
    int rc;
    zmq_msg_t msg;
    void *watch;
    unsigned long elapsed;
    unsigned long throughput;
    double megabits;

    //if (argc != 4) {
    //    printf ("usage: local_thr <bind-to> <message-size> <message-count>\n");
    //    return 1;
    //}
    //bind_to = argv [1];
    message_size = argc>3 ? atoi (argv [3]) : 50;
    message_count = argc>2 ? atoi (argv [2]) : 100;
	bind_to = argc>1 ? argv [1] : "tcp://127.0.0.1:2015";
	const char *bind_toSend = "tcp://*:2016";

    ctx = zmq_init (1);
    if (!ctx) {
        printf ("error in zmq_init: %s\n", zmq_strerror (errno));
        return -1;
    }

    receive = zmq_socket (ctx, ZMQ_PULL);
    if (!receive) {
        printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
        return -1;
    }

    //  Add your socket options here.
    //  For example ZMQ_RATE, ZMQ_RECOVERY_IVL and ZMQ_MCAST_LOOP for PGM.

    rc = zmq_bind (receive, bind_to);
    if (rc != 0) {
        printf ("error in zmq_bind: %s\n", zmq_strerror (errno));
        return -1;
    }


	send = zmq_socket (ctx, ZMQ_PUB);
	if (!send) {
		printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
		return -1;
	}

	//  Add your socket options here.
	//  For example ZMQ_RATE, ZMQ_RECOVERY_IVL and ZMQ_MCAST_LOOP for PGM.

	rc = zmq_bind (send, bind_toSend);
	if (rc != 0) {
		printf ("error in zmq_bind: %s\n", zmq_strerror (errno));
		return -1;
	}
	int s1;
	size_t r1 = sizeof(int);
	zmq_getsockopt (send, ZMQ_SNDHWM, &s1, &r1);
	zmq_getsockopt (send, ZMQ_SNDBUF, &s1, &r1);
	int jsendsize2 = 100000 * 200;
	zmq_setsockopt(send, ZMQ_SNDBUF, &jsendsize2, sizeof(jsendsize2));
	zmq_getsockopt (send, ZMQ_SNDBUF, &s1, &r1);
	
	rc = zmq_msg_init (&msg);
    if (rc != 0) {
        printf ("error in zmq_msg_init: %s\n", zmq_strerror (errno));
        return -1;
    }
    watch = zmq_stopwatch_start ();
	zmq_pollitem_t items [] = {
		{ receive, 0, ZMQ_POLLIN, 0 }
	};

	int iCount = 0;
	while (true) {
		zmq_msg_t message;
		zmq_poll (items, 1, -1);
		if (items [0].revents & ZMQ_POLLIN) {
			zmq_msg_init (&message);
			zmq_recvmsg (receive, &message, 0);
			int isize = zmq_msg_size(&message);
			char *val = (char *)(zmq_msg_data(&message));
			val[isize] = '\0';
			rc = zmq_sendmsg (send, &message, 0);
			if (rc < 0) {
				printf ("error in zmq_sendmsg: %s\n", zmq_strerror (errno));
				return -1;
			}
			zmq_msg_close (&message);
			printf ("%d %s\n", iCount++, val);
		}
	}


	//while (true)
	//{
	//	rc = zmq_sendmsg (send, &msg, 0);
	//	if (rc < 0) {
	//		printf ("error in zmq_sendmsg: %s\n", zmq_strerror (errno));
	//		return -1;
	//	}
	//	Sleep(200);
	//}
    elapsed = zmq_stopwatch_stop (watch);
    if (elapsed == 0)
        elapsed = 1;

    rc = zmq_msg_close (&msg);
    if (rc != 0) {
        printf ("error in zmq_msg_close: %s\n", zmq_strerror (errno));
        return -1;
    }

    throughput = (unsigned long)
        ((double) message_count / (double) elapsed * 1000000);
    megabits = (double) (throughput * message_size * 8) / 1000000;

    printf ("message size: %d [B]\n", (int) message_size);
    printf ("message count: %d\n", (int) message_count);
    printf ("mean throughput: %d [msg/s]\n", (int) throughput);
    printf ("mean throughput: %.3f [Mb/s]\n", (double) megabits);

    rc = zmq_close (receive);
    if (rc != 0) {
        printf ("error in zmq_close: %s\n", zmq_strerror (errno));
        return -1;
    }

	rc = zmq_close (send);
	if (rc != 0) {
		printf ("error in zmq_close: %s\n", zmq_strerror (errno));
		return -1;
	}
    rc = zmq_term (ctx);
    if (rc != 0) {
        printf ("error in zmq_term: %s\n", zmq_strerror (errno));
        return -1;
    }

    return 0;
}
