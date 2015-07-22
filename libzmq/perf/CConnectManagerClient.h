#pragma once

#include "CConnectManagerBase.h"
#define  WIN32_LEAN_AND_MEAN
#include <msgpack.hpp>
#include <zmq.h>
#include <sstream>

class CMQSqlQuery
{
public:
	CMQSqlQuery();
	~CMQSqlQuery();
public:
	bool isActive() const;
	void InitState();
	void Reset();
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
	zmq_msg_t reply;
};

//class CMQSqRecord
//{
//public:
//	CMQSqRecord();
//	~CMQSqRecord();
//	CMQSqRecord(CMQSqlQuery *sqlQuery);
//	//struct CAdapterString
//	//{
//	//	CAdapterString(const std::string &str) : m_strVal(str);
//	//	std::string toStdString()
//	//	{
//	//		return m_strVal;
//	//	}
//	//	std::string m_strVal;
//	//};
//	std::string fieldName(int iIndex);
//
//public:
//	CMQSqlQuery *m_MQSqlQuery;
//	bool m_bMQQuery;
//};

typedef std::auto_ptr<CMQSqlQuery> MQSqlQuery;
class CMQSqlDML
{
public:
	CMQSqlDML();
	~CMQSqlDML();
public:
	void InitState();
public:
	zmq_msg_t reply;
	bool m_bResult;
};

class CConnectManagerClient : public CConnectManagerBase
{
public:
	CConnectManagerClient(void *prequester);
	~CConnectManagerClient();
	
public:
	CMQSqlQuery *ExecQuery(const std::string &strSql);
	bool ExecCommand(const std::string &strSql, int iCmdType = SCT_DML);
	std::stringstream *OpenStream(const std::string &tablename, const std::string &keyName, 
		int id, const std::string &blobName, bool bWrite);
	bool StartBatchOpenStream(const std::string &tablename, const std::string &keyName, 
		const std::string &strCondition, const std::string &blobName);
	std::stringstream *NextBlobStream(int &id);
	bool GetTables(std::vector<std::string> &tableNames);
	CMQSqlQuery *GetTableInfo(const std::string &tableName);

	bool CloseStream();

	bool BeginTrans();
	bool CommitTrans();
	bool RollbackTrans();

private:
	void *requester;
	std::stringstream *m_pIOStream;
	bool m_bStreamWrite;
	std::string m_strWriteSql;
	MQSqlQuery m_BatchQuery;
	bool m_bBatchOpenBlobStream;

};

