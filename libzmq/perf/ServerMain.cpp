#include "ServerMain.h"
#define  WIN32_LEAN_AND_MEAN
#include <msgpack.hpp>
#pragma warning(disable: 4996)

//#include "CppSQLite3.h"
#include <iostream>
#include "CConnectManagerServer.h"
using namespace std;

#include <zmq.h>
#include <stdio.h>
#include <string.h>
int main_rep () 
{
	void *context = zmq_init (1);
	// Socket to talk to clients
	void *responder = zmq_socket (context, ZMQ_REP);
	zmq_bind (responder, "tcp://*:5555");
	while (1) {
		// Wait for next request from client
		zmq_msg_t request;
		zmq_msg_init (&request);
		zmq_recvmsg (responder, &request, 0);
		printf ("Received request: [%s]\n",
			(char *) zmq_msg_data (&request));
		zmq_msg_close (&request);
		// Do some 'work'
		//zmq_sleep (1);
		// Send reply back to client
		zmq_msg_t reply;
		zmq_msg_init_size (&reply, 6);
		memcpy ((void *) zmq_msg_data (&reply), "World", 6);
		zmq_sendmsg (responder, &reply, 0);
		zmq_msg_close (&reply);
	}
	zmq_term (context);
	return 0;
}
int main_rep_file_trans () 
{
	void *context = zmq_init (1);
	// Socket to talk to clients
	void *responder = zmq_socket (context, ZMQ_REP);
	zmq_bind (responder, "tcp://*:5555");
	int iStart, iCount;
	int iStep = 1000;
	int iTotalCount = 3 * 1024 * 1024;
	char *str = new char [iTotalCount];
	const char *strFile = "c:\\Column.bmp";
	const char *strFile2 = "c:\\python279.chm";
	const char *strFile3 = "c:\\cygQtWebKit-4.dll";
	int start = 0;
	int endtime = 0;
	int totaltime = 0;
	while (1) {
		// Wait for next request from client
		zmq_msg_t request;
		zmq_msg_init (&request);
		zmq_recvmsg (responder, &request, 0);
		char *pStr = (char *) zmq_msg_data (&request);
		//printf ("Received request: [%s]\n",
		//	pStr );
		sscanf(pStr, "%d %d", &iStart, &iCount);
		zmq_msg_close (&request);
		int iReadCount = 0;//(iEnd - iStart) * iStep;
			start = GetTickCount();
		if (FILE *fp = fopen(strFile3, "rb"))
		{
			if (-1 != fseek(fp, iStart, SEEK_SET))
				iReadCount = fread(str, 1, iCount, fp);
			fclose(fp);
		}
		endtime = GetTickCount();
		totaltime += endtime - start;
		if (iReadCount == 0)
		{
			printf ("Read: %d ms\n", totaltime);
			totaltime = 0;
		}
		// Do some 'work'
		//zmq_sleep (1);
		// Send reply back to client
		zmq_msg_t reply;
		zmq_msg_init_size (&reply, iReadCount);
		memcpy ((void *) zmq_msg_data (&reply), str, iReadCount);
		zmq_sendmsg (responder, &reply, 0);
		zmq_msg_close (&reply);
	}
	delete[] str;
	zmq_term (context);
	return 0;
}

int main_rep_file_trans_pack () 
{
	void *context = zmq_init (1);
	// Socket to talk to clients
	void *responder = zmq_socket (context, ZMQ_REP);
	zmq_bind (responder, "tcp://*:5555");
	int iStart, iCount;
	int iStep = 1000;
	int iTotalCount = 5 * 1024 * 1024;
	char *str = new char [iTotalCount];
	const char *strFile = "c:\\Column.bmp";
	const char *strFile2 = "c:\\python279.chm";
	const char *strFile3 = "c:\\cygQtWebKit-4.dll";
	int start = 0;
	int endtime = 0;
	int totaltime = 0;
	while (1) {
		// Wait for next request from client
		zmq_msg_t request;
		zmq_msg_init (&request);
		zmq_recvmsg (responder, &request, 0);
		char *pStr = (char *) zmq_msg_data (&request);

		sscanf(pStr, "%d %d", &iStart, &iCount);
		zmq_msg_close (&request);
		int iReadCount = 0;//(iEnd - iStart) * iStep;
			start = GetTickCount();
		if (FILE *fp = fopen(strFile3, "rb"))
		{
			if (-1 != fseek(fp, iStart, SEEK_SET))
				iReadCount = fread(str, 1, iCount, fp);
			fclose(fp);
		}
		endtime = GetTickCount();
		totaltime += endtime - start;
		if (iReadCount == 0)
		{
			printf ("Read: %d ms\n", totaltime);
			totaltime = 0;
		}
		// Do some 'work'
		//zmq_sleep (1);
		// Send reply back to client
		//pack str
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> packer(sbuf);
		//msgpack::pack(sbuf, msgpack::type::ext_ref(str, iReadCount));
		packer.pack_ext(iReadCount, 77);
		packer.pack_ext_body(str, iReadCount);

		iReadCount = sbuf.size();
		zmq_msg_t reply;
		zmq_msg_init_size (&reply, iReadCount);
		memcpy ((void *) zmq_msg_data (&reply), sbuf.data(), iReadCount);
		zmq_sendmsg (responder, &reply, 0);
		zmq_msg_close (&reply);
	}
	delete[] str;
	zmq_term (context);
	return 0;
}

int main_rep_db_trans_pack () 
{
	void *context = zmq_init (1);
	// Socket to talk to clients
	void *responder = zmq_socket (context, ZMQ_REP);
	zmq_bind (responder, "tcp://*:5555");
	//int iStart, iCount;
	int iStep = 1000;
	int iTotalCount = 5 * 1024 * 1024;
	char *str = new char [iTotalCount];
	const char *strFile = "c:\\S2P2.gmdsl";
	int start = 0;
	int endtime = 0;
	int totaltime = 0;
	CConnectManagerServer conManager;
	conManager.ConnectDB(strFile);
	if (!conManager.m_bActive)
		return 0;

	while (1) {
		// Wait for next request from client
		zmq_msg_t request;
		zmq_msg_init (&request);
		zmq_recvmsg (responder, &request, 0);
		const char *pStr = (const char *) zmq_msg_data (&request);
		size_t iSize = zmq_msg_size(&request);
		//std::string strQuery = pStr;
		start = GetTickCount();
		conManager.ProcessCommand(pStr, iSize);
		zmq_msg_close (&request);

		int iReadCount = 0;//(iEnd - iStart) * iStep;

	
		iReadCount = conManager.sbuf.size();
		zmq_msg_t reply;
		zmq_msg_init_size (&reply, iReadCount);
		memcpy ((void *) zmq_msg_data (&reply), conManager.sbuf.data(), iReadCount);
		zmq_sendmsg (responder, &reply, 0);
		zmq_msg_close (&reply);
		int stop = GetTickCount();
		printf(" %d(ms)\n", stop - start);
	}
	delete[] str;
	zmq_term (context);
	return 0;
}
