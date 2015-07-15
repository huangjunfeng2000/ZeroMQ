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
#include <string.h>
#include "ClientMain.h"

int main (int argc, char *argv [])
{
	//main_req_file_trans();
	//main_req_file_trans_pack();
	main_req_db_trans_pack();

	return 0;
    const char *connect_to;
    int message_count;
    int message_size;
    void *ctx;
    void *send;
	void *receive;
    int rc;
    int i;
    zmq_msg_t msg;

    //if (argc != 4) {
    //    printf ("usage: remote_thr <connect-to> <message-size> "
    //        "<message-count>\n");
    //    return 1;
    //}
    //connect_to = argv [1];
    //message_size = atoi (argv [2]);
    //message_count = atoi (argv [3]);

	connect_to = argc>1 ? argv [1] : "tcp://127.0.0.1:2015";
	message_size = argc>3 ? atoi (argv [3]) : 50;
	message_count = argc>2 ? atoi (argv [2]) : 1000;

    ctx = zmq_init (1);
    if (!ctx) {
        printf ("error in zmq_init: %send\n", zmq_strerror (errno));
        return -1;
    }

    send = zmq_socket (ctx, ZMQ_PUSH);
    if (!send) {
        printf ("error in zmq_socket: %send\n", zmq_strerror (errno));
        return -1;
    }

    //  Add your socket options here.
    //  For example ZMQ_RATE, ZMQ_RECOVERY_IVL and ZMQ_MCAST_LOOP for PGM.

    rc = zmq_connect (send, connect_to);
    if (rc != 0) {
        printf ("error in zmq_connect: %s\n", zmq_strerror (errno));
        return -1;
    }

	receive = zmq_socket (ctx, ZMQ_SUB);
	if (!receive) {
		printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
		return -1;
	}
	zmq_setsockopt(receive, ZMQ_SUBSCRIBE, "", 0);
	int s1;
	size_t r1 = sizeof(int);
	zmq_getsockopt (send, ZMQ_RCVHWM, &s1, &r1);
	zmq_getsockopt (send, ZMQ_RCVBUF, &s1, &r1);
	int jsendsize2 = 100000 * 200;
	zmq_setsockopt(send, ZMQ_RCVBUF, &jsendsize2, sizeof(jsendsize2));
	zmq_getsockopt (send, ZMQ_RCVBUF, &s1, &r1);


	//  Add your socket options here.
	//  For example ZMQ_RATE, ZMQ_RECOVERY_IVL and ZMQ_MCAST_LOOP for PGM.
	const char *connect_to2 = "tcp://127.0.0.1:2016";
	rc = zmq_connect (receive, connect_to2);
	if (rc != 0) {
		printf ("error in zmq_connect: %s\n", zmq_strerror (errno));
		return -1;
	}
	Sleep(1000);
    for (i = 0; i != message_count; i++) {
		zmq_msg_close(&msg);
        
        //if (rc != 0) {
        //    printf ("error in zmq_msg_init_size: %s\n", zmq_strerror (errno));
        //    return -1;
        //}
		//printf("send %d: \n", i);
		char str[100];
		memset(str, 0, 100);
		sprintf(str, "val: %-3d", i+1);
		sprintf(str, "%s", "Lock:20485:-100:0:290774416");
		rc = zmq_msg_init_size (&msg, strlen(str));
		memcpy(zmq_msg_data(&msg), str, strlen(str));
        rc = zmq_sendmsg (send, &msg, 0);
        if (rc < 0) {
            printf ("error in zmq_sendmsg: %s\n", zmq_strerror (errno));
            return -1;
        }
        rc = zmq_msg_close (&msg);
        if (rc != 0) {
            printf ("error in zmq_msg_close: %s\n", zmq_strerror (errno));
            return -1;
        }
    }  

	zmq_pollitem_t items [] = {
		{ receive, 0, ZMQ_POLLIN, 0 }
	};
	int nCount = 1;
	int nTryTimes = 0;
	while (true){//nTryTimes<3) {
		zmq_msg_t message;
		zmq_poll (items, 1, 1000);
		if (items [0].revents & ZMQ_POLLIN) {
			zmq_msg_init (&message);
			zmq_recvmsg (receive, &message, 0);
			char *val = (char *)(zmq_msg_data(&message));
			printf ("%d in zmq_recvmsg: %s\n", nCount++, val);
			zmq_msg_close (&message);
			nTryTimes = 0;
		}
		else
		{
			nTryTimes++;
		}
	}

 //   for (i = 0; i != message_count-1; i++) {
	//	printf ("in zmq_recvmsg: %d\n", i);
	//	zmq_msg_t msg2;
	//	rc = zmq_msg_init  (&msg2);
	//	rc = zmq_recvmsg (receive, &msg2, 0);
	//	if (rc < 0) {
	//		printf ("error in zmq_recvmsg: %s\n", zmq_strerror (errno));
	//		return -1;
	//	}
	//	int message_size = zmq_msg_size (&msg2);
	//}
    rc = zmq_close (receive);
    if (rc != 0) {
        printf ("error in zmq_close: %send\n", zmq_strerror (errno));
        return -1;
    }
	rc = zmq_close (send);
	if (rc != 0) {
		printf ("error in zmq_close: %send\n", zmq_strerror (errno));
		return -1;
	}
    rc = zmq_term (ctx);
    if (rc != 0) {
        printf ("error in zmq_term: %send\n", zmq_strerror (errno));
        return -1;
    }

    return 0;
}
