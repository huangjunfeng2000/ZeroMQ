#pragma once

#include "CConnectManagerBase.h"
#define  WIN32_LEAN_AND_MEAN
#include <msgpack.hpp>
#include <zmq.h>
#include "CppSQLite3.h"

//int main_req ();
//int main_req_file_trans ();
//int main_req_file_trans_pack ();
//int main_req_db_trans_pack ();

//class CMQSqlQuery
//{
//public:
//	CMQSqlQuery();
//	~CMQSqlQuery();
//public:
//	bool isActive() const;
//	void InitState();
//	bool next();
//	const msgpack::object &value(int i);
//	const msgpack::object &nextvalue();
//	void printval(const msgpack::object &msgObj);
//
//public:
//
//	struct CMQMsgInfo
//	{
//		CMQMsgInfo();
//		const char *pVal;
//		size_t iSize;	//msg size
//		int iFieldCount;	
//		bool m_bActive;
//		std::size_t off;
//		int iCurIndex;
//		int iRowIndex;
//
//		msgpack::unpacked msg;
//		msgpack::object nullObject;
//		void Init(zmq_msg_t &reply);
//		const msgpack::object &next();
//	};
//
//	CMQMsgInfo msgInfo;
//	//std::size_t off;
//	zmq_msg_t reply;
//};

class CConnectManagerServer : public CConnectManagerBase
{
public:
	CConnectManagerServer();
	~CConnectManagerServer();
	bool ConnectDB(const std::string &strDB);
	bool ProcessCommand(const std::string &strCmd);

public:
	bool ExecQuery(const std::string &strSql);

	bool m_bActive;
	msgpack::sbuffer sbuf;
private:
	CppSQLite3DB db;

};

