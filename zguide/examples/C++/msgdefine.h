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

	GMT_W_CLOSED,

	GMT_C_EDIT_DB = 0x1<<8,			//修改数据(客户端）
	GMT_C_ADD_DB = 0x1<<9,			//增加数据(客户端）
	GMT_C_DELETE_DB = 0x1<<10,		//删除数据(客户端）
	GMT_C_CLOSE_PROJECT_DB = 0x1<<11,	//单用户关闭工区数据库
	GMT_C_CLOSE_ALL_DB = 0x1<<12,	//所有用户关闭工区数据库

	GMT_C_EDIT_FILE = 0x1<<13,			//修改文件(客户端）
	GMT_C_READ_FILE = 0x1<<14,			//读文件(客户端）
	GMT_C_EDIT_OPTION_FILE = 0x1<<15,	//修改配置文件(客户端）
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
enum GDS_PROJECT_STATE
{
	GPS_RUNNING,	//运行
	GPS_SUSPEND,	//暂停
	GPS_STOP		//停止
};

enum GDS_GUID_TYPE
{
	GGT_OPTION_FILE,
	GGT_USER_FILE

};

class CMsgBase
{
public:
	virtual int Type() const;
	bool PraseString(const std::string &str);

	bool ReadFromJsonNode(const Json::Value &rootnode);
	bool WriteToJsonNode(Json::Value &rootnode);
	std::string ToString(GDS_OPERATOR_TYPE  type = GOT_NULL);
	mutable GDS_OPERATOR_TYPE m_iOperatorType;
protected:
	CMsgBase();
	virtual ~CMsgBase();
	virtual bool DoReadFromJsonNode(const Json::Value &rootnode) = 0;
	virtual bool DoWriteToJsonNode(Json::Value &rootnode) = 0;
};

class CMsgGeneralCommand : public CMsgBase
{
public:
	virtual int Type()  const{return m_iType;}
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
	virtual int Type() const {return GMT_CONNECT_SERVER;}
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
	virtual int Type() const {return GMT_PROJECT_INFO;}
	CMsgProjectInfo();
	CMsgProjectInfo(const std::string &strFile);
	std::string m_strName;
	std::string m_strPath;
	std::string m_strUUID;
	GDS_PROJECT_TYPE m_projectType;
	bool DoReadFromJsonNode(const Json::Value &rootnode);
	bool DoWriteToJsonNode(Json::Value &node);
	std::string ServiecName();
	void SetProjectName(const std::string &strFile);

};
class CMsgProjectList : public CMsgBase
{
public:
	CMsgProjectList();
	virtual int Type() const {return GMT_SERVER_INFO;}
	bool DoReadFromJsonNode(const Json::Value &rootnode);
	virtual bool DoWriteToJsonNode(Json::Value &rootnode);
	std::vector<CMsgProjectInfo> m_vecProInfo;
};

class CMsgUUID : public CMsgBase
{
public:
	CMsgUUID();
	CMsgUUID(int iMsgType, const std::string &uuid);
	virtual int Type() const {return m_iMsgType;}
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
	virtual int Type() const {return m_iMsgType;}
	bool DoReadFromJsonNode(const Json::Value &rootnode);
	virtual bool DoWriteToJsonNode(Json::Value &rootnode);
public:
	int m_iMsgType;
	std::string m_strInfo;
};
//CMsgAddDB
class CMsgAddDB : public CMsgBase
{
public:
	CMsgAddDB() {}
	CMsgAddDB(int iTableType, /*int iType, int iID, */const std::string &strUserID = "A");
	virtual int Type() const {return GMT_C_ADD_DB;}
	bool DoReadFromJsonNode(const Json::Value &rootnode);
	virtual bool DoWriteToJsonNode(Json::Value &rootnode);

public:
	int m_iTableType;
	int m_iType, m_iID;
	std::string m_strUUID;
};
//CMsgEditDB GMT_C_DELETE_DB  
class CMsgEditDB : public CMsgBase
{
public:
	CMsgEditDB() {}
	CMsgEditDB(int iMsgType, int iType, int iID, const std::string &strUserID = "A");
	virtual int Type() const {return m_iMsgType;}
	bool DoReadFromJsonNode(const Json::Value &rootnode);
	virtual bool DoWriteToJsonNode(Json::Value &rootnode);
public:
	int m_iMsgType;
	int m_iType, m_iID;
	std::string m_strUUID;
};

//CMsgEditFile
class CMsgFileBase : public CMsgBase
{
public:
	CMsgFileBase() {}
	CMsgFileBase(int iMsgType) : m_iMsgType(iMsgType) {}
	virtual int Type() const {return m_iMsgType;}
	bool DoReadFromJsonNode(const Json::Value &rootnode);
	virtual bool DoWriteToJsonNode(Json::Value &rootnode);

public:
	int m_iMsgType;
	std::string m_strFile;
	std::string m_strUUID;
};

//CMsgEditFile
class CMsgEditFile : public CMsgFileBase
{
public:
	CMsgEditFile() {}
	CMsgEditFile(int iMsgType, const std::string &strFile, const std::string &strUserID = "A");
};

//CMsgEditOption
class CMsgEditOption : public CMsgFileBase
{
public:
	CMsgEditOption() {}
	CMsgEditOption(int iOptionType, const std::string &strUserID = "A");
};




#endif
