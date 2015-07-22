#pragma once

#include "CConnectManagerBase.h"
#define  WIN32_LEAN_AND_MEAN
#include <msgpack.hpp>
#include <zmq.h>
#include "CppSQLite3.h"

class CConnectManagerServer : public CConnectManagerBase
{
public:
	CConnectManagerServer();
	~CConnectManagerServer();
	bool ConnectDB(const std::string &strDB);
	bool ProcessCommand(const char * strCmd, size_t iSize);

public:
	bool ExecQuery(const std::string &strSql);
	bool ExecCommand(const std::string &strSql);
	bool ExecCommandBin(const std::string &strSql, const unsigned char *pStr, size_t iSize);

	bool m_bActive;
	msgpack::sbuffer sbuf;
private:
	CppSQLite3DB db;

};

