#include "CConnectManagerClient.h"
#define  WIN32_LEAN_AND_MEAN
#include <msgpack.hpp>
//
// Hello World client
// Connects REQ socket to tcp://localhost:5555
// Sends "Hello" to server, expects "World" back
//
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#define bExport true
#pragma warning (disable:4996)
using namespace std;
CMQSqlQuery::CMQMsgInfo::CMQMsgInfo()
{
	iFieldCount = 0;
	off = 0;
	iSize = 0;
	pVal = NULL;
	iCurIndex = 0;
	iRowIndex = 0;
}
void CMQSqlQuery::Reset()
{
	msgInfo.Init(reply);
}
void CMQSqlQuery::CMQMsgInfo::Init(zmq_msg_t &reply)
{
	pVal = (const char *)zmq_msg_data(&reply);
	iSize = zmq_msg_size(&reply);
	m_bActive = iSize > 0;
	iFieldCount = 0;
	off = 0;
	if (m_bActive)
	{
		msgpack::unpack(msg, pVal, iSize, off);
		iFieldCount = msg.get().as<int>();
	}
	m_bActive = m_bActive && iFieldCount > 0;
}
const msgpack::object &CMQSqlQuery::CMQMsgInfo::next()
{
	if (off < iSize)
	{
		if (iCurIndex == 0 && bExport)
			printf("\n--------------------------V%-3d--------------------------\n", ++iRowIndex);
		msgpack::unpack(msg, pVal, iSize, off);
		iCurIndex = ++iCurIndex % iFieldCount;
		m_bActive = off < iSize;
		return msg.get();
	}
	iCurIndex = 0;
	m_bActive = false;
	return nullObject;
}


CMQSqlQuery::CMQSqlQuery()
{
	zmq_msg_init (&reply);
}
CMQSqlQuery::~CMQSqlQuery()
{
	zmq_msg_close (&reply);
}
bool CMQSqlQuery::isActive() const
{
	return msgInfo.m_bActive;
}
void CMQSqlQuery::InitState()
{
	msgInfo.Init(reply);
}

bool CMQSqlQuery::next()
{
	bool bValid = true;
	while (msgInfo.iCurIndex && bValid)
	{
		const msgpack::object &obj = msgInfo.next();
		bValid = !obj.is_nil();
	}
	return bValid && msgInfo.m_bActive;
}

const msgpack::object &CMQSqlQuery::value(int i)
{
	assert(msgInfo.iCurIndex == i);
	return msgInfo.next();
}
const msgpack::object &CMQSqlQuery::nextvalue()
{
	return msgInfo.next();
}
void CMQSqlQuery::printval(const msgpack::object &msgObj)
{
	switch(msgObj.type)
	{
	case msgpack::type::BOOLEAN:
		{
			bool tempVal = msgObj.as<bool>();
			if (bExport) 
				printf("%d ", tempVal);
		}
		break;
	case msgpack::type::POSITIVE_INTEGER:
	case msgpack::type::NEGATIVE_INTEGER:
		{
			int tempVal = msgObj.as<int>();
			if (bExport)
				printf("%d ", tempVal);
		}
		break;
	case msgpack::type::FLOAT:
		{
			float tempVal = msgObj.as<float>();
			if (bExport)
				printf("%.2f ", tempVal);
		}
		break;
	case msgpack::type::STR:
		{
			int iSize = msgObj.via.str.size;
			const char *tempVal = msgObj.via.str.ptr;
			std::string str(tempVal, iSize);
			str = UTF_82ASCII(str);
			if (bExport)
				printf("%s ", iSize ? str.c_str() : "NL");
		}
		break;
	case msgpack::type::BIN:
		{
			const char *tempVal = msgObj.via.bin.ptr;
			int iSize = msgObj.via.bin.size;
			if (bExport)
				printf("BLOB(%d) ", iSize);
		}
		break;
	case msgpack::type::EXT:
		{
			const char *tempVal = msgObj.via.ext.data();
			int iSize = msgObj.via.bin.size;
			if (bExport)
				printf("EXT(%d) ", "EXT");
		}
		break;
	case msgpack::type::NIL:
		assert(false);
		break;
	default:
		assert(false);
		break;
	}
}
////////////////////////////////////////////////////////
//CMQSqRecord::CMQSqRecord()
//{
//	m_MQSqlQuery = NULL; 
//	m_bMQQuery = false;
//}
//CMQSqRecord::~CMQSqRecord()
//{
//
//}
//CMQSqRecord::CMQSqRecord(CMQSqlQuery *sqlQuery) : m_MQSqlQuery(sqlQuery), m_bMQQuery(true)
//{
//
//}
////////////////////////////////////////////////////////
CMQSqlDML::CMQSqlDML()
{
	m_bResult = false;
	zmq_msg_init (&reply);
}
CMQSqlDML::~CMQSqlDML()
{
	zmq_msg_close (&reply);
}

void CMQSqlDML::InitState()
{
	const char *pVal = (const char *)zmq_msg_data(&reply);
	size_t iSize = zmq_msg_size(&reply);
	bool m_bActive = iSize > 0;
	size_t off = 0;
	if (m_bActive)
	{
		msgpack::unpacked msg;
		msgpack::unpack(msg, pVal, iSize, off);
		m_bResult = msg.get().as<bool>();
	}
}
//////////////
CConnectManagerClient::CConnectManagerClient(void *prequester)
{
	m_bStreamWrite = false;
	requester = prequester;
	m_bBatchOpenBlobStream = false;
	m_pIOStream = new std::stringstream(stringstream::in | stringstream::out | stringstream::binary);
}
CConnectManagerClient::~CConnectManagerClient()
{
	delete m_pIOStream;
}
std::string CombineSql(int iType, const std::string &strSql)
{
	std::string strQuery = ASCII2UTF_8(strSql);
	char str[10];
	memset(str, 0, 10);
	itoa(iType, str, 10);
	return std::string(str) + " " + strQuery;
}

CMQSqlQuery *CConnectManagerClient::ExecQuery(const std::string &strSql)
{
	static int iQurIndex = 0;
	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> packer(sbuf);
	packer.pack(int(SCT_QUERY));
	packer.pack(strSql);
	//std::string strQuery = CombineSql(SCT_QUERY, strSql);//ASCII2UTF_8(strSql);
	std::cout <<endl <<  "Query(" << ++iQurIndex <<"): " << "(" << strSql.substr(0, 60) << ")";

	zmq_msg_t request;
	//zmq_msg_init_data (&request, (void *)strQuery.c_str(), strQuery.length() + 1, NULL, NULL);
	zmq_msg_init_data (&request, (void *)sbuf.data(), sbuf.size(), NULL, NULL);

	zmq_sendmsg (requester, &request, 0);
	zmq_msg_close (&request);

	CMQSqlQuery *pSqlQuery = new CMQSqlQuery;
	zmq_recvmsg (requester, &pSqlQuery->reply, 0);
	pSqlQuery->InitState();
	return pSqlQuery;
}

bool CConnectManagerClient::ExecCommand(const std::string &strSql, int iCmdType)
{
	static int iQurIndex = 0;
	//std::string strQuery = CombineSql(SCT_DML, strSql);//ASCII2UTF_8(strSql);
	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> packer(sbuf);
	packer.pack(int(iCmdType));
	packer.pack(strSql);
	if (iCmdType == SCT_DML_BIN)
	{
		m_pIOStream->seekp(0, ios::end);
		int iLen = m_pIOStream->tellp();
		iLen = iLen < 0 ? 0 : iLen;
		m_pIOStream->seekp(0, ios::beg);
		//const char *tempVal = (const char *)m_pIOStream->rdbuf();
		packer.pack_bin(iLen);
		packer.pack_bin_body(m_pIOStream->str().c_str(), iLen);
	}
	std::cout <<endl <<  "DML(" << ++iQurIndex <<"): " << "(" << strSql.substr(0, 60) << ")";
	zmq_msg_t request;
	zmq_msg_init_data (&request, (void *)sbuf.data(), sbuf.size(), NULL, NULL);

	//zmq_msg_init_data (&request, (void *)strQuery.c_str(), strQuery.length() + 1, NULL, NULL);
	zmq_sendmsg (requester, &request, 0);
	zmq_msg_close (&request);

	CMQSqlDML pSqlDML;
	zmq_recvmsg (requester, &pSqlDML.reply, 0);
	pSqlDML.InitState();
	std::cout << endl <<  "Result: " << pSqlDML.m_bResult;
	return pSqlDML.m_bResult;
}

bool CConnectManagerClient::BeginTrans()
{
	return ExecCommand("begin transaction;");
}
bool CConnectManagerClient::CommitTrans()
{
	return ExecCommand("commit transaction;");
}
bool CConnectManagerClient::RollbackTrans()
{
	return ExecCommand("rollback transaction;");
}
std::stringstream *CConnectManagerClient::OpenStream(const std::string &tablename, const std::string &keyName, 
							  int id, const std::string &blobName, bool bWrite)
{
	char strSql[100];
	memset(strSql, 0, 100);
	if (!bWrite)
	{
		sprintf(strSql, "SELECT %s, %s FROM %s WHERE %s=%d", keyName.c_str(), blobName.c_str(), tablename.c_str(), keyName.c_str(), id);
		MQSqlQuery pQury(ExecQuery(strSql));
		if (pQury.get() && pQury->next())
		{
			assert(2 == pQury->msgInfo.iFieldCount);
			const msgpack::object &objID = pQury->nextvalue();
			assert(objID.as<int>() == id);
			const msgpack::object &objStr = pQury->nextvalue();
			const char *tempVal = objStr.via.bin.ptr;
			int iSize = objStr.via.bin.size;
			m_pIOStream->str("");
			m_pIOStream->write(tempVal, iSize);
			//std::cout <<endl << "OpenStream(read): " << std::string(strSql).substr(0, 60);
			std::cout << "Value: " << setw(-60) << m_pIOStream->str().substr(0, 60);

			return m_pIOStream;
		}
	}
	else
	{
		assert(!m_bStreamWrite);
		m_bStreamWrite = true;
		sprintf(strSql, "UPDATE %s SET %s = ? WHERE %s=%d", tablename.c_str(), blobName.c_str(), keyName.c_str(), id);
		m_strWriteSql = strSql;
		m_pIOStream->str("");
		return m_pIOStream;
	}
	return NULL;
}
bool CConnectManagerClient::StartBatchOpenStream(const std::string &tablename, const std::string &keyName, 
						  const std::string &strCondition, const std::string &blobName)
{
	std::string strSql = "Select " + keyName + ", " + blobName + " " + "From " + tablename + " where " +  strCondition + " order by " + keyName;
	if (CMQSqlQuery *pQuery = ExecQuery(strSql))
	{
		m_BatchQuery = MQSqlQuery(pQuery);
		if (pQuery->isActive())
			m_bBatchOpenBlobStream = true;
		else
			m_BatchQuery.reset();
	}
	return m_bBatchOpenBlobStream;
}
std::stringstream *CConnectManagerClient::NextBlobStream(int &id)
{
	if (m_BatchQuery.get() && m_BatchQuery->next())
	{
		assert(2 == m_BatchQuery->msgInfo.iFieldCount);
		const msgpack::object &objID = m_BatchQuery->nextvalue();
		id = objID.as<int>();
		const msgpack::object &objStr = m_BatchQuery->nextvalue();
		const char *tempVal = objStr.via.bin.ptr;
		int iSize = objStr.via.bin.size;
		m_pIOStream->str("");
		m_pIOStream->write(tempVal, iSize);
		std::cout << "NextBlobStream: " << id << " " << setw(-60) << m_pIOStream->str().substr(0, 60);
		return m_pIOStream;
	}
	return NULL;
}
bool CConnectManagerClient::CloseStream()
{
	if (m_bStreamWrite)
	{
		assert(!m_strWriteSql.empty());
		m_bStreamWrite = false;
		return ExecCommand(m_strWriteSql, SCT_DML_BIN);
	}
	if (m_bBatchOpenBlobStream)
	{
		m_BatchQuery.reset();
		m_bBatchOpenBlobStream = false;
	}
	return true;
}
bool CConnectManagerClient::GetTables(std::vector<std::string> &tableNames)
{
	tableNames.resize(0);
	std::string strSql = "select name from sqlite_master where type='table'";
	MQSqlQuery pQury(ExecQuery(strSql));
	while (pQury.get() && pQury->next())
	{
		assert(1 == pQury->msgInfo.iFieldCount);
		const msgpack::object &objStr = pQury->nextvalue();
		int iSize = objStr.via.str.size;
		const char *tempVal = objStr.via.str.ptr;
		std::string str(tempVal, iSize);
		str = UTF_82ASCII(str);
		tableNames.push_back(str);
	}
	return NULL != pQury.get();
}
CMQSqlQuery *CConnectManagerClient::GetTableInfo(const std::string &tableName)
{
	char strSql[50];
	memset(strSql, 0, 50);
	sprintf(strSql, "PRAGMA table_info(\"%s\");", tableName.c_str());
	return ExecQuery(strSql);
}
