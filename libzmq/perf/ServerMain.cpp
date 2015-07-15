#include "ServerMain.h"
#define  WIN32_LEAN_AND_MEAN
#include <msgpack.hpp>
#pragma warning(disable: 4996)

//#include "CppSQLite3.h"
#include <iostream>
#include "CConnectManagerServer.h"
using namespace std;

//#include <gtest/gtest.h>
//#include <sstream>
//
// Hello World server
// Binds REP socket to tcp://*:5555
// Expects "Hello" from client, replies with "World"
//
#include <zmq.h>
#include <stdio.h>
//#include <unistd.h>
#include <string.h>
int main_rep () {
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
int main_rep_file_trans () {
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

int main_rep_file_trans_pack () {
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
		//pack str
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> packer(sbuf);
		//msgpack::pack(sbuf, msgpack::type::ext_ref(str, iReadCount));
		packer.pack_ext(iReadCount, 77);
		packer.pack_ext_body(str, iReadCount);

		//Test unpack
		//msgpack::unpacked msgPack;
		//msgpack::unpack(msgPack, (char *)sbuf.data(), sbuf.size());
		////const msgpack::object& packObj = msgPack.get();
		//msgpack::type::ext_ref val2;
		//msgPack.get().convert(val2);
		//assert(val2.size() == iReadCount);
		//assert(val2.type() == 77);
		//int iReadCount2 = val2.size();


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

int main_rep_db_trans_pack () {
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
	//CppSQLite3DB db;
	//try
	//{
	//	db.open(strFile);
	//}
	//catch (CppSQLite3Exception * e)
	//{
	//	printf("%s\n", e->errorMessage());
	//	return 0;
	//}
	//catch (...)
	//{
	//	return 0;
	//}

	while (1) {
		// Wait for next request from client
		zmq_msg_t request;
		zmq_msg_init (&request);
		zmq_recvmsg (responder, &request, 0);
		char *pStr = (char *) zmq_msg_data (&request);
		//printf ("Received request: [%s]\n",
		//	pStr );
		std::string strQuery = pStr;
		//strQuery = ASCII2UTF_8(strQuery);
		zmq_msg_close (&request);

		conManager.ProcessCommand(strQuery);

		//CppSQLite3Query q = db.execQuery(strQuery.c_str());
		//int iFieldCount = q.numFields();
		//std::vector<int> vecFieldType(iFieldCount);
		//for (int fld = 0; fld < iFieldCount; fld++)
		//{
		//	//std::cout << q.fieldName(fld) << "(" << q.fieldDeclType(fld) << ")|";
		//	vecFieldType[fld] = q.fieldDataType(fld);
		//}
		////cout << endl;
		//std::cout << "ExecQuery: FieldCount " << iFieldCount;

		//msgpack::sbuffer sbuf;
		//msgpack::packer<msgpack::sbuffer> packer(sbuf);
		//packer.pack(iFieldCount);
		//int iRowCount = 0;
		//while (!q.eof())
		//{
		//	++iRowCount;
		//	for (int i=0; i<iFieldCount; ++i)
		//	{
		//		switch(vecFieldType[i])
		//		{
		//		case SQLITE_INTEGER:
		//			{
		//				int tempVal = q.getIntField(i, 0);
		//				packer.pack(tempVal);
		//			}
		//			break;
		//		case SQLITE_FLOAT:
		//			{
		//				float tempVal = q.getFloatField(i, 0);
		//				packer.pack(tempVal);
		//			}
		//			break;
		//		case SQLITE_TEXT:
		//			{
		//				const unsigned char *tempVal = (const unsigned char *)q.getStringField(i);
		//				//std::string str((const char *)tempVal);
		//				//str = UTF_82ASCII(str);
		//				//packer.pack(str);
		//				packer.pack((const char *)tempVal);
		//			}
		//			break;
		//		case SQLITE_NULL:
		//			{
		//				const char *tempVal = "";//q.getStringField(i);
		//				packer.pack(tempVal);
		//			}
		//			break;
		//		case SQLITE_BLOB:
		//			{
		//				int iLen = 0;
		//				const char *tempVal = (const char *)q.getBlobField(i, iLen);
		//				packer.pack_bin(iLen);
		//				packer.pack_bin_body(tempVal, iLen);
		//				//packer.pack_ext(iLen, 77);
		//				//packer.pack_ext_body(tempVal, iLen);
		//			}
		//			break;
		//		default:
		//			assert(false);
		//			break;
		//		}
		//	}
		//	//std::cout << q.fieldValue(0) << "|";
		//	//std::cout << q.fieldValue(1) << "|" << endl;
		//	q.nextRow();
		//}
		//std::cout << " RowCount: " << iRowCount << " (" << strQuery.substr(0, 30) << "...)" << endl;

		//sscanf(pStr, "%d %d", &iStart, &iCount);

		int iReadCount = 0;//(iEnd - iStart) * iStep;
			start = GetTickCount();
		//if (FILE *fp = fopen(strFile3, "rb"))
		//{
		//	if (-1 != fseek(fp, iStart, SEEK_SET))
		//		iReadCount = fread(str, 1, iCount, fp);
		//	fclose(fp);
		//}
		//endtime = GetTickCount();
		//totaltime += endtime - start;
		//if (iReadCount == 0)
		//{
		//	printf ("Read: %d ms\n", totaltime);
		//	totaltime = 0;
		//}
		// Do some 'work'
		//zmq_sleep (1);
		// Send reply back to client
		//pack str

		//msgpack::pack(sbuf, msgpack::type::ext_ref(str, iReadCount));


		//Test unpack
		//msgpack::unpacked msgPack;
		//msgpack::unpack(msgPack, (char *)sbuf.data(), sbuf.size());
		////const msgpack::object& packObj = msgPack.get();
		//msgpack::type::ext_ref val2;
		//msgPack.get().convert(val2);
		//assert(val2.size() == iReadCount);
		//assert(val2.type() == 77);
		//int iReadCount2 = val2.size();


		iReadCount = conManager.sbuf.size();
		zmq_msg_t reply;
		zmq_msg_init_size (&reply, iReadCount);
		memcpy ((void *) zmq_msg_data (&reply), conManager.sbuf.data(), iReadCount);
		zmq_sendmsg (responder, &reply, 0);
		zmq_msg_close (&reply);
	}
	delete[] str;
	zmq_term (context);
	return 0;
}
//
// Weather update server
// Binds PUB socket to tcp://*:5556
// Publishes random weather updates
//
//#include "zhelpers.h"
//int main_pub () {
//	// Prepare our context and publisher
//	void *context = zmq_init (1);
//	void *publisher = zmq_socket (context, ZMQ_PUB);
//	zmq_bind (publisher, "tcp://*:5556");
//	zmq_bind (publisher, "ipc://weather.ipc");
//		// Initialize random number generator
//	srandom ((unsigned) time (NULL));
//	while (1) {
//		// Get values that will fool the boss
//		int zipcode, temperature, relhumidity;
//		zipcode = within (100000);
//		temperature = within (215) - 80;
//		relhumidity = within (50) + 10;
//		// Send message to all subscribers
//		char update [20];
//		sprintf (update, "%05d %d %d", zipcode, temperature, relhumidity);
//		s_send (publisher, update);
//	}
//	zmq_close (publisher);
//	zmq_term (context);
//	return 0;
//}