#pragma once

#include "CConnectManagerBase.h"
#define  WIN32_LEAN_AND_MEAN
#include <msgpack.hpp>
#include <zmq.h>

//int main_req ();
//int main_req_file_trans ();
//int main_req_file_trans_pack ();
//int main_req_db_trans_pack ();

class CMQSqlQuery
{
public:
	CMQSqlQuery();
	~CMQSqlQuery();
public:
	bool isActive() const;
	void InitState();
	bool next();
	const msgpack::object &value(int i);
	const msgpack::object &nextvalue();
	void printval(const msgpack::object &msgObj);

public:

	struct CMQMsgInfo
	{
		CMQMsgInfo();
		const char *pVal;
		size_t iSize;	//msg size
		int iFieldCount;	
		bool m_bActive;
		std::size_t off;
		int iCurIndex;
		int iRowIndex;

		msgpack::unpacked msg;
		msgpack::object nullObject;
		void Init(zmq_msg_t &reply);
		const msgpack::object &next();
	};

	CMQMsgInfo msgInfo;
	//std::size_t off;
	zmq_msg_t reply;
};

class CConnectManagerClient : public CConnectManagerBase
{
public:
	CConnectManagerClient(void *prequester);
	~CConnectManagerClient();
	
public:
	CMQSqlQuery *ExecQuery(const std::string &strSql);

private:
	void *requester;

};

