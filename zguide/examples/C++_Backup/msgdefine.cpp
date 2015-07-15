#include "msgdefine.h"
#include "objbase.h"
#include "jsonAPI.h"

std::string newGUID()
{
	char buf[128];
	GUID guid;
	if (S_OK == ::CoCreateGuid(&guid))
	{
		sprintf(buf, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"
			, guid.Data1
			, guid.Data2
			, guid.Data3
			, guid.Data4[0], guid.Data4[1]
		, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
		, guid.Data4[6], guid.Data4[7]);
	}
	return buf;
}

CMsgBase::CMsgBase()
{

}
CMsgBase::~CMsgBase()
{

}
int CMsgBase::Type()
{
	return 0;
}
bool CMsgBase::PraseString(const std::string &str)
{
	Json::Value node = JSON_GetRootNode();
	bool bRes = JSON_Prase(str, node);
	return bRes && DoReadFromJsonNode(node);
}

std::string CMsgBase::ToString(GDS_OPERATOR_TYPE  type)
{
	Json::Value node = JSON_GetRootNode();
	m_iOperatorType = type;
	WriteToJsonNode(node);
	return JSON_ToString(node);
}
bool CMsgBase::ReadFromJsonNode(const Json::Value &rootnode)
{
	int iType;
	JSON_GetAttributeValue(rootnode, iType, "OperatorType");
	m_iOperatorType = GDS_OPERATOR_TYPE(iType);
	return DoReadFromJsonNode(rootnode);
}
bool CMsgBase::WriteToJsonNode(Json::Value &rootnode)
{
	JSON_SetAttributeValue(rootnode, m_iOperatorType, "OperatorType");
	return DoWriteToJsonNode(rootnode);
}
//CMsgGeneralCommand
CMsgGeneralCommand::CMsgGeneralCommand(int iType) : m_iType(iType)
{

}
bool CMsgGeneralCommand::DoWriteToJsonNode(Json::Value &node)
{
	JSON_SetNodeType(node, m_iType);

	return true;
}

bool CMsgGeneralCommand::DoReadFromJsonNode(const Json::Value &rootnode)
{
	m_iType = JSON_GetNodeType(rootnode);
	return true;
}

//CMsgClient
CMsgConnectInfo::CMsgConnectInfo(const std::string &strUser, const std::string &strPassWord) 
	: m_strUser(strUser), m_strPassWord(strPassWord)
{

}
CMsgConnectInfo::CMsgConnectInfo()
{
	m_strUser = "m_strUser";
	m_strPassWord = "m_strPassWord";
}

bool CMsgConnectInfo::DoWriteToJsonNode(Json::Value &node)
{
	JSON_SetNodeType(node, GMT_CONNECT_SERVER);
	JSON_SetAttributeValue(node, m_strUser, "User");
	JSON_SetAttributeValue(node, m_strPassWord, "Pass");
	return true;
}

bool CMsgConnectInfo::DoReadFromJsonNode(const Json::Value &rootnode)
{
	return 	JSON_GetAttributeValue(rootnode, m_strUser, "User")
		 && JSON_GetAttributeValue(rootnode, m_strPassWord, "Pass");
}
//CMsgProjectInfo();
CMsgProjectInfo::CMsgProjectInfo() : m_projectType(GPT_PROJECT_NONE)
{
	
}
CMsgProjectInfo::CMsgProjectInfo(const std::string &strFile)
{
	char drive[_MAX_DRIVE];  
	char dir[_MAX_DIR];  
	char fname[_MAX_FNAME];  
	char ext[_MAX_EXT];  
	_splitpath( strFile.c_str(), drive, dir, fname, ext );
	strupr(ext);

	std::string strExt = strFile.substr(strFile.length()-4, 4);
	m_projectType = GPT_PROJECT_NONE;
	if (strcmp(ext, ".UNI") == 0)
		m_projectType = GPT_PROJECT_MAP;
	else if (strcmp(ext, ".GMDSL") == 0)
		m_projectType = GPT_PROJECT_MODEL;

	m_strName = fname;
	m_strPath = strFile;
	m_strUUID = newGUID();
}
bool CMsgProjectInfo::DoReadFromJsonNode(const Json::Value &rootnode)
{
	int iType = 0;
	bool bFlag=	JSON_GetAttributeValue(rootnode, iType, "m_projectType")
		&& JSON_GetAttributeValue(rootnode, m_strName, "m_strName")
		&& JSON_GetAttributeValue(rootnode, m_strPath, "m_strPath")
		&& JSON_GetAttributeValue(rootnode, m_strUUID, "m_strUUID");
	m_projectType = (GDS_PROJECT_TYPE)iType;
	return bFlag;
}
bool CMsgProjectInfo::DoWriteToJsonNode(Json::Value &node)
{
	JSON_SetAttributeValue(node, (int)m_projectType, "m_projectType");
	JSON_SetAttributeValue(node, m_strName, "m_strName");
	JSON_SetAttributeValue(node, m_strPath, "m_strPath");
	JSON_SetAttributeValue(node, m_strUUID, "m_strUUID");
	return true;
}
std::string CMsgProjectInfo::ServiecName()
{
	return m_projectType==GPT_PROJECT_MAP ? "Map" : (m_projectType ==GPT_PROJECT_MODEL ? "Model" : "None");
}
//CMsgProjectList
CMsgProjectList::CMsgProjectList()
{

}
bool CMsgProjectList::DoWriteToJsonNode(Json::Value &rootnode)
{
	JSON_SetNodeType(rootnode, GMT_SERVER_INFO);
	Json::Value ProjectNode;// = node["ProjectList"];  
	for (size_t i=0; i<m_vecProInfo.size(); ++i)
	{
		Json::Value subNode;
		m_vecProInfo[i].WriteToJsonNode(subNode);
		ProjectNode.append(subNode);
	}
	rootnode["ProjectList"] = ProjectNode;
	return true;
}
bool CMsgProjectList::DoReadFromJsonNode(const Json::Value &rootnode)
{
	const Json::Value &ProjectNode = rootnode["ProjectList"]; 
	m_vecProInfo.resize(ProjectNode.size());
	for (size_t i=0; i<m_vecProInfo.size(); ++i)
	{
		m_vecProInfo[i].ReadFromJsonNode(ProjectNode[i]);
	}
	return true;
}
//CMsgUUID
CMsgUUID::CMsgUUID()
{

}
CMsgUUID::CMsgUUID(int iMsgType, const std::string &uuid) : m_iMsgType(iMsgType), m_strUUID(uuid) 
{

}
bool CMsgUUID::DoWriteToJsonNode(Json::Value &rootnode)
{
	JSON_SetNodeType(rootnode, m_iMsgType);
	JSON_SetAttributeValue(rootnode, m_strUUID, "UUID");
	return true;
}
bool CMsgUUID::DoReadFromJsonNode(const Json::Value &rootnode)
{
	m_iMsgType = JSON_GetNodeType(rootnode);
	bool bFlag=	JSON_GetAttributeValue(rootnode, m_strUUID, "UUID");
	return bFlag;
}

//CMsgResult
CMsgResult::CMsgResult(int iMsgType, const std::string &info) : m_iMsgType(iMsgType), m_strInfo(info) 
{

}
bool CMsgResult::DoWriteToJsonNode(Json::Value &rootnode)
{
	JSON_SetNodeType(rootnode, m_iMsgType);
	JSON_SetAttributeValue(rootnode, m_strInfo, "m_strInfo");
	return true;
}
bool CMsgResult::DoReadFromJsonNode(const Json::Value &rootnode)
{
	m_iMsgType = JSON_GetNodeType(rootnode);
	bool bFlag=	JSON_GetAttributeValue(rootnode, m_strInfo, "m_strInfo");
	return bFlag;
}
//CMsgEditDB
CMsgEditDB::CMsgEditDB(int iMsgType, int iType, int iID) : m_iType(iType), m_iID(iID) 
{

}
bool CMsgEditDB::DoWriteToJsonNode(Json::Value &rootnode)
{
	JSON_SetNodeType(rootnode, m_iMsgType);
	JSON_SetAttributeValue(rootnode, m_iType, "m_iType");
	JSON_SetAttributeValue(rootnode, m_iID, "iID");
	JSON_SetAttributeValue(rootnode, m_strUUID, "m_strUUID");
	return true;
}
bool CMsgEditDB::DoReadFromJsonNode(const Json::Value &rootnode)
{
	m_iMsgType = JSON_GetNodeType(rootnode);
	bool bFlag=	JSON_GetAttributeValue(rootnode, m_iType, "m_iType")
	&& JSON_GetAttributeValue(rootnode, m_iID, "m_iID")
	&& JSON_GetAttributeValue(rootnode, m_strUUID, "m_strUUID");
	return bFlag;
}
