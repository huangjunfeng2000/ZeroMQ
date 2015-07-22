#include "ClientMain.h"
#define  WIN32_LEAN_AND_MEAN
#include <msgpack.hpp>
#include "CConnectManagerClient.h"

// Hello World client
// Connects REQ socket to tcp://localhost:5555
// Sends "Hello" to server, expects "World" back
//
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#pragma warning (disable:4996)

using namespace std;
#include <conio.h>
int main_req () {
	void *context = zmq_init (1);
	// Socket to talk to server
	printf ("Connecting to hello world server...\n");
	void *requester = zmq_socket (context, ZMQ_REQ);
	zmq_connect (requester, "tcp://localhost:5555");
	int request_nbr;
	for (request_nbr = 0; request_nbr != 1000; request_nbr++) {
		zmq_msg_t request;
		zmq_msg_init_data (&request, "Hello", 6, NULL, NULL);
		printf ("Sending request %d...\n", request_nbr);
		zmq_sendmsg (requester, &request, 0);
		zmq_msg_close (&request);
		zmq_msg_t reply;
		zmq_msg_init (&reply);
		zmq_recvmsg (requester, &reply, 0);
		printf ("Received reply %d: [%s]\n", request_nbr,
			(char *) zmq_msg_data (&reply));
		zmq_msg_close (&reply);
	}
	zmq_close (requester);
	zmq_term (context);
	return 0;
}

int main_req_file_trans () {
	void *context = zmq_init (1);
	// Socket to talk to server
	printf ("Connecting to hello world server...\n");
	void *requester = zmq_socket (context, ZMQ_REQ);
	zmq_connect (requester, "tcp://localhost:5555");
	int request_nbr = 0;
		const char *strFile = "c:\\Column2.bmp";


	for (request_nbr = 0; request_nbr < 400; request_nbr++) 
	{
		int start = GetTickCount();
		char str[30];
		//for (int i=0; i<100; ++i)
		int i=0;
		int iStart = 0;
		int iCount = 3000 + (request_nbr +1 ) * 5000;
		int iWriteCount = 0;
		while(true)
		{
			memset(str, '\0', 30);
			sprintf_s(str, "%d %d ", iStart, iCount);
			iStart += iCount;
			++i;
			zmq_msg_t request;
			zmq_msg_init_data (&request, str, strlen(str), NULL, NULL);
			zmq_sendmsg (requester, &request, 0);
			zmq_msg_close (&request);
			zmq_msg_t reply;
			zmq_msg_init (&reply);
			zmq_recvmsg (requester, &reply, 0);
			int iReadCount = zmq_msg_size(&reply);

			if (iReadCount==0)
				break;

				int start1 = GetTickCount();
				void *pVal = zmq_msg_data(&reply);
				if (FILE *fp = fopen(strFile, i==1 ? "wb" : "ab"))
				{
					fseek(fp, 0, SEEK_END);
					fwrite(pVal, iReadCount, 1, fp);
					fclose(fp);
				}
				int stop1 = GetTickCount();
				iWriteCount += stop1 - start1;

			zmq_msg_close (&reply);
		}
		int stop = GetTickCount();
		printf("time: %d %d total:%d ms write:%d ms\n", i, iCount, stop - start, iWriteCount);
	}
	zmq_close (requester);
	zmq_term (context);

	getch();
	return 0;
}

int main_req_file_trans_pack () {
	void *context = zmq_init (1);
	// Socket to talk to server
	printf ("Connecting to hello world server...\n");
	void *requester = zmq_socket (context, ZMQ_REQ);
	zmq_connect (requester, "tcp://localhost:5555");
	int request_nbr = 0;
	const char *strFile = "c:\\Column2.bmp";


	for (request_nbr = 100; request_nbr < 600; request_nbr++) 
	{
		int start = GetTickCount();
		char str[30];
		//for (int i=0; i<100; ++i)
		int i=0;
		int iStart = 0;
		int iCount = 3000 + (request_nbr +1 ) * 5000;
		int iWriteCount = 0;
		while(true)
		{
			memset(str, '\0', 30);
			sprintf_s(str, "%d %d ", iStart, iCount);
			iStart += iCount;
			++i;
			zmq_msg_t request;
			zmq_msg_init_data (&request, str, strlen(str), NULL, NULL);
			//printf ("Sending request %s...\n", str);
			zmq_sendmsg (requester, &request, 0);
			zmq_msg_close (&request);
			zmq_msg_t reply;
			zmq_msg_init (&reply);
			zmq_recvmsg (requester, &reply, 0);
			int iReadCount = zmq_msg_size(&reply);

			int start1 = GetTickCount();
			void *pVal = zmq_msg_data(&reply);
			int iSize = zmq_msg_size(&reply);
			std::size_t off = 0;
			msgpack::unpacked msgPack;
			msgpack::unpack(msgPack, (char *)pVal, iSize);
			const msgpack::object& packObj = msgPack.get();
			msgpack::type::ext_ref val2;
			packObj.convert(val2);
			//char *pResStr = packObj.as<char *>();
			int iReadCount2 = val2.size();
			iReadCount = iReadCount2;
			int iValType = val2.type();
			if (iReadCount==0)
				break;
			pVal = (void *)val2.data();//msgPack.buffer();
			if (FILE *fp = fopen(strFile, i==1 ? "wb" : "ab"))
			//if (FILE *fp = fopen(strFile, i>0 ? "wb" : "ab"))
			{
				fseek(fp, 0, SEEK_END);
				fwrite(pVal, iReadCount, 1, fp);
				fclose(fp);
			}
			int stop1 = GetTickCount();
			iWriteCount += stop1 - start1;
			//}

			zmq_msg_close (&reply);
		}
		int stop = GetTickCount();
		printf("time: %d %d total:%d ms write:%d ms\n", i, iCount, stop - start, iWriteCount);
	}
	zmq_close (requester);
	zmq_term (context);

	getch();
	return 0;
}
struct CSqlInfo
{
	SQL_CMD_TYPE m_Type;
	std::string m_strSql;
	CSqlInfo(SQL_CMD_TYPE type, const std::string &strql) : m_Type(type), m_strSql(strql){}
};
int main_req_db_trans_pack () {
	void *context = zmq_init (1);
	// Socket to talk to server
	printf ("Connecting to hello world server...\n");
	void *requester = zmq_socket (context, ZMQ_REQ);
	zmq_connect (requester, "tcp://localhost:5555");
	int request_nbr = 0;

	enum {sqlCount = 16};
	CSqlInfo strQur[sqlCount] = {
		CSqlInfo(SCT_QUERY, "select * from tbl_wellinfo;"),
		CSqlInfo(SCT_DML, "begin transaction;"),
		CSqlInfo(SCT_DML, "update tbl_wellinfo set wellname=\"A\""),
		CSqlInfo(SCT_DML, "commit transaction;"),
		CSqlInfo(SCT_DML, "begin transaction;"),
		CSqlInfo(SCT_DML, "update tbl_wellinfo set wellname=\"B\""),
		CSqlInfo(SCT_DML, "rollback transaction;"),
		CSqlInfo(SCT_QUERY, "select max(wellid) as maxid from tbl_wellinfo;"),
		CSqlInfo(SCT_DML_BIN, "WriteStream"),
		CSqlInfo(SCT_DML_BIN, "ReadStream"),
		CSqlInfo(SCT_DML_BIN, "BatchReadStream"),
		CSqlInfo(SCT_QUERY, "select name from sqlite_master where type='table';"),
		CSqlInfo(SCT_QUERY, "select * from sqlite_master where type='table';"),
		CSqlInfo(SCT_QUERY, "PRAGMA table_info(\"tbl_wellinfo\");"),
		CSqlInfo(SCT_QUERY, "GetTables"),
		CSqlInfo(SCT_QUERY, "GetTableInfo"),


	};
	CConnectManagerClient cmClient(requester);
	bool bExport = false;

	for (int i = 0; i < sqlCount; i++) 
	{
		int start = GetTickCount();
		CSqlInfo &item = strQur[i];
		switch(item.m_Type)
		{
		case SCT_QUERY:
			{
				CMQSqlQuery *pQury = NULL;
				if (item.m_strSql == "GetTables")
				{
					std::vector<std::string> tableNames;
					cmClient.GetTables(tableNames);
				}
				else if (item.m_strSql == "GetTableInfo")
				{
					pQury = cmClient.GetTableInfo("tbl_wellinfo");
					std::vector<std::string> colNames;
					while(pQury && pQury->next())
					{
						const msgpack::object &obj = pQury->nextvalue();
						const msgpack::object &objName = pQury->nextvalue();
						assert(msgpack::type::STR == objName.type);
						int iSize = objName.via.str.size;
						const char *tempVal = objName.via.str.ptr;
						std::string str(tempVal, iSize);
						colNames.push_back(str);
						//pQury->printval(obj);
					}
					pQury->Reset();
				}
				else
				{
					pQury = cmClient.ExecQuery(item.m_strSql);
				}
				
				while(pQury && pQury->next())
				{
					for (int j=0; j<pQury->msgInfo.iFieldCount; ++j)
					{
						const msgpack::object &obj = pQury->nextvalue();
						pQury->printval(obj);
					}
				}
				delete pQury;
			}
			break;
		case SCT_DML:
			{
				bool bResult = cmClient.ExecCommand(item.m_strSql);
			}
			break;
		case SCT_DML_BIN:
			{
				std::string strTest = "DEF E";
				strTest.resize(100000, 'X');

				if (item.m_strSql == "WriteStream")
				{
					//write
					std::stringstream *pStr = cmClient.OpenStream("tbl_wellinfo", "wellid", 2, "data", true);
					pStr->write(strTest.c_str(), strTest.size());
					cmClient.CloseStream();
				}
				else if (item.m_strSql == "ReadStream")
				{
					//read
					std::stringstream *pStr = cmClient.OpenStream("tbl_wellinfo", "wellid", 2, "data", false);
					assert(pStr->str() == strTest);
					cmClient.CloseStream();
				}
				else if (item.m_strSql == "BatchReadStream")
				{
					if (cmClient.StartBatchOpenStream("tbl_wellinfo", "wellid", "1=1", "data"))
					{
						
						int id;
						while(std::stringstream *pStr = cmClient.NextBlobStream(id))
							;
						//std::stringstream *pStr = cmClient.NextBlobStream(id);
					}

					cmClient.CloseStream();
				}
			}
			break;
		default:
			assert(false);
			break;
		}

		int stop = GetTickCount();
		printf("\ntotal:%d ms\n", stop - start);
	}
	
	zmq_close (requester);
	zmq_term (context);

	getch();
	return 0;
}