#include "CConnectManagerServer.h"
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

#pragma warning (disable: 4996)
#define bExport true

using namespace std;
//////////////
CConnectManagerServer::CConnectManagerServer()
{
	m_bActive = false;
}
CConnectManagerServer::~CConnectManagerServer()
{

}

bool CConnectManagerServer::ConnectDB(const std::string &strDB)
{
	try
	{
		db.open(strDB.c_str());
		m_bActive = true;
	}
	catch (CppSQLite3Exception * e)
	{
		printf("%s\n", e->errorMessage());
		m_bActive = false;
	}
	return m_bActive;
}

std::string DeCombineSql(int &iType, const std::string &strSql) 
{
	iType = SCT_NONE;
	sscanf(strSql.c_str(), "%d", &iType);
	std::string strResult = strSql.substr(strSql.find_first_of(" ") + 1, strSql.length());
	return strResult;
}
bool CConnectManagerServer::ProcessCommand(const char * strCmd, size_t iSize)
{
	int iType;
	//std::string strsql = DeCombineSql(iType, strCmd);
	std::string strsql;
	size_t off = 0;
	msgpack::unpacked msg;
	if (iSize > 0)
	{
		msgpack::unpack(msg, strCmd, iSize, off);
		iType = msg.get().as<int>();
	}
	if (off < iSize)
	{
		msgpack::unpack(msg, strCmd, iSize, off);
		strsql = msg.get().as<std::string>();
	}

	switch(iType)
	{
	case SCT_QUERY:
		return ExecQuery(strsql);
	case SCT_DML:
		return ExecCommand(strsql);
	case SCT_DML_BIN:
		{
			if (off < iSize)
			{
				msgpack::unpack(msg, strCmd, iSize, off);
				const msgpack::object &msgObj = msg.get();
				if (msgpack::type::BIN == msgObj.type )
				{
					const char *tempVal = msgObj.via.bin.ptr;
					int iSize = msgObj.via.bin.size;
					return ExecCommandBin(strsql, (const unsigned char *)tempVal, iSize);
				}
			}
		}
		return false;
	default:
		return false;
	}
}
bool CConnectManagerServer::ExecQuery(const std::string &strSql)
{
	CppSQLite3Query q = db.execQuery(strSql.c_str());
	int iFieldCount = q.numFields();
	std::vector<int> vecFieldType(iFieldCount);
	for (int fld = 0; fld < iFieldCount; fld++)
	{
		//std::cout << q.fieldName(fld) << "(" << q.fieldDeclType(fld) << ")|";
		vecFieldType[fld] = q.fieldDataType(fld);
	}
	//cout << endl;
	std::cout << "ExecQuery: FieldCount " << iFieldCount;

	//msgpack::sbuffer sbuf;
	sbuf.clear();
	msgpack::packer<msgpack::sbuffer> packer(sbuf);
	packer.pack(iFieldCount);
	int iRowCount = 0;
	while (!q.eof())
	{
		++iRowCount;
		for (int i=0; i<iFieldCount; ++i)
		{
			switch(vecFieldType[i])
			{
			case SQLITE_INTEGER:
				{
					int tempVal = q.getIntField(i, 0);
					packer.pack(tempVal);
				}
				break;
			case SQLITE_FLOAT:
				{
					double tempVal = q.getFloatField(i, 0);
					packer.pack(tempVal);
				}
				break;
			case SQLITE_TEXT:
				{
					const unsigned char *tempVal = (const unsigned char *)q.getStringField(i);
					packer.pack((const char *)tempVal);
				}
				break;
			case SQLITE_NULL:
				{
					const char *tempVal = "";//q.getStringField(i);
					packer.pack(tempVal);
				}
				break;
			case SQLITE_BLOB:
				{
					int iLen = 0;
					const char *tempVal = (const char *)q.getBlobField(i, iLen);
					packer.pack_bin(iLen);
					packer.pack_bin_body(tempVal, iLen);
					//packer.pack_ext(iLen, 77);
					//packer.pack_ext_body(tempVal, iLen);
				}
				break;
			default:
				assert(false);
				break;
			}
		}
		q.nextRow();
	}
	std::cout << " RowCount: " << iRowCount << " (" << strSql.substr(0, 30) << "...)";
	return true;
}

bool CConnectManagerServer::ExecCommand(const std::string &strSql)
{
	sbuf.clear();
	msgpack::packer<msgpack::sbuffer> packer(sbuf);
	
	bool bResult = true;
	int iEffectRowCount = 0;
	try
	{
		iEffectRowCount = db.execDML(strSql.c_str());
	}
	catch (...)
	{
		bResult = false;
	}
	packer.pack(bResult);
	std::cout << "ExecCommand Result: " << iEffectRowCount << " " << bResult << " (" << strSql.substr(0, 30) << "...)";
	return bResult;
}

bool CConnectManagerServer::ExecCommandBin(const std::string &strSql, const unsigned char *pStr, size_t iSize)
{
	sbuf.clear();
	msgpack::packer<msgpack::sbuffer> packer(sbuf);

	bool bResult = true;
	int iEffectRowCount = 0;
	try
	{
		CppSQLite3Statement stmt = db.compileStatement(strSql.c_str());
		stmt.bind(1, pStr, iSize);
		stmt.execDML();
		stmt.reset();
	}
	catch (...)
	{
		bResult = false;
	}
	packer.pack(bResult);
	std::cout << "ExecCommand Result: " << iEffectRowCount << " " << bResult << " (" << strSql.substr(0, 30) << "...)";
	return bResult;
}