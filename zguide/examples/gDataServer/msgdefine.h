#ifndef __MSGDEFINE_HPP_INCLUDED__
#define __MSGDEFINE_HPP_INCLUDED__

#include "jsonAPI.h"

#define MSG_CONNECT_FLAG "HELLO_GDS_SERVER"
#define MSG_VIRTUAL_SERVICE "GDS.VIRTUAL_SERVICE"

static char *gds_msg_connect [] = {
	NULL, (char*)"READY", (char*)"REQUEST", (char*)"REPLY", (char*)"HEARTBEAT", (char*)"DISCONNECT"
};


//return result
#define MSG_SUCCESS	"GDS_SUCCESS"
#define MSG_FAILURE	"GDS_FAILURE"

std::string newGUID();


enum GDS_MSG_TYPE
{
	GMT_NULL			= 0,
	GMT_CONNECT_SERVER	= 1,
	GMT_PROJECT_INFO = 2,
	GMT_SERVER_INFO = 3,
	GMT_OPEN_PROJECT	= 4,
	GMT_S_SUCCESS,		//服务器返回信息
	GMT_S_FAILURE,
	GMT_C_EDIT_DB		//修改数据

};
enum GDS_PROJECT_TYPE
{
	GPT_PROJECT_NONE,
	GPT_PROJECT_MAP,
	GPT_PROJECT_MODEL
};
enum GDS_OPERATOR_TYPE
{
	GOT_NULL,
	GOT_LOCK,
	GOT_UNLOCK
};

class CMsgBase
{
public:
	virtual int Type();
	bool PraseString(const std::string &str);

	bool ReadFromJsonNode(const Json::Value &rootnode);
	bool WriteToJsonNode(Json::Value &rootnode);
	std::string ToString(GDS_OPERATOR_TYPE  type = GOT_NULL);
	GDS_OPERATOR_TYPE m_iOperatorType;
protected:
	CMsgBase();
	virtual ~CMsgBase();
	virtual bool DoReadFromJsonNode(const Json::Value &rootnode) = 0;
	virtual bool DoWriteToJsonNode(Json::Value &rootnode) = 0;
};

class CMsgGeneralCommand : public CMsgBase
{
public:
	CMsgGeneralCommand(int iType);

	bool DoReadFromJsonNode(const Json::Value &rootnode);
	virtual bool DoWriteToJsonNode(Json::Value &rootnode);
private:
	int m_iType;

};
//
class CMsgConnectInfo : public CMsgBase
{
public:
	std::string m_strUser;
	std::string m_strPassWord;
	CMsgConnectInfo();
	CMsgConnectInfo(const std::string &strUser, const std::string &strPassWord);
	bool DoReadFromJsonNode(const Json::Value &rootnode);
	virtual bool DoWriteToJsonNode(Json::Value &rootnode);
};
//
class CMsgProjectInfo : public CMsgBase
{
public:
	CMsgProjectInfo();
	CMsgProjectInfo(const std::string &strFile);
	std::string m_strName;
	std::string m_strPath;
	std::string m_strUUID;
	GDS_PROJECT_TYPE m_projectType;
	bool DoReadFromJsonNode(const Json::Value &rootnode);
	bool DoWriteToJsonNode(Json::Value &node);
	std::string ServiecName();

};
class CMsgProjectList : public CMsgBase
{
public:
	CMsgProjectList();

	bool DoReadFromJsonNode(const Json::Value &rootnode);
	virtual bool DoWriteToJsonNode(Json::Value &rootnode);
	std::vector<CMsgProjectInfo> m_vecProInfo;
};

class CMsgUUID : public CMsgBase
{
public:
	CMsgUUID();
	CMsgUUID(int iMsgType, const std::string &uuid);

	bool DoReadFromJsonNode(const Json::Value &rootnode);
	virtual bool DoWriteToJsonNode(Json::Value &rootnode);
public:
	int m_iMsgType;
	std::string m_strUUID;
};

//CMsgResult
class CMsgResult : public CMsgBase
{
public:
	CMsgResult() {}
	CMsgResult(int iMsgType, const std::string &info);

	bool DoReadFromJsonNode(const Json::Value &rootnode);
	virtual bool DoWriteToJsonNode(Json::Value &rootnode);
public:
	int m_iMsgType;
	std::string m_strInfo;
};

//CMsgEditDB
class CMsgEditDB : public CMsgBase
{
public:
	CMsgEditDB() {}
	CMsgEditDB(int iMsgType, int iType, int iID);

	bool DoReadFromJsonNode(const Json::Value &rootnode);
	virtual bool DoWriteToJsonNode(Json::Value &rootnode);
public:
	int m_iMsgType;
	int m_iType, m_iID;
	std::string m_strUUID;
};

#endif
