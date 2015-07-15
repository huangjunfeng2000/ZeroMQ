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

std::string GetGUID(int iIndex)
{
	static std::string strGuidAry[] = {
			"{D7467E00-1F97-45A8-B478-E5E6AFDC67D0}",
			"{5842E832-28E6-46B7-9203-02A97ED41AA9}",
			"{4EB41482-3123-43F8-86D1-C121B17E1FCF}",
			"{AE117603-153C-4ED7-BBE7-2136C8E77D25}",
			"{F5EFF2FA-570D-4DA7-8BF8-20A2879CC450}",
			"{B8A2EC2A-CB4D-4991-9137-7A9773CA500E}",
			"{6A66140F-852D-41E5-BFE8-6CCFC04AD9ED}",
			"{C889DE09-C326-4819-BFB7-3A0E57809C59}",
			"{6B7ACC7A-AF1F-4241-80B8-F3E79C0CC14A}",
			"{7BD01DF8-6882-4AD5-8375-EA215309704D}"
	};
	ASSERT_C(iIndex>=0 && iIndex < 10);
	return strGuidAry[iIndex];
}

CMsgBase::CMsgBase()
{

}
CMsgBase::~CMsgBase()
{

}
int CMsgBase::Type() const
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
	SetProjectName(strFile);
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
void CMsgProjectInfo::SetProjectName(const std::string &strFile)
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
//CMsgAddDB
CMsgAddDB::CMsgAddDB(int iTableType,/* int iType, int iID, */const std::string &strUserID) : /*m_iType(iType), m_iID(iID),*/ m_iTableType(iTableType), m_strUUID(strUserID)
{

}

bool CMsgAddDB::DoWriteToJsonNode(Json::Value &rootnode)
{
	JSON_SetNodeType(rootnode, m_iTableType);
	JSON_SetAttributeValue(rootnode, m_iType, "m_iType");
	JSON_SetAttributeValue(rootnode, m_iID, "m_iID");
	JSON_SetAttributeValue(rootnode, m_strUUID, "m_strUUID");
	return true;
}
bool CMsgAddDB::DoReadFromJsonNode(const Json::Value &rootnode)
{
	m_iTableType = JSON_GetNodeType(rootnode);
	bool bFlag=	JSON_GetAttributeValue(rootnode, m_iType, "m_iType")
		&& JSON_GetAttributeValue(rootnode, m_iID, "m_iID")
		&& JSON_GetAttributeValue(rootnode, m_strUUID, "m_strUUID");
	return bFlag;
}


//CMsgEditDB
CMsgEditDB::CMsgEditDB(int iMsgType, int iType, int iID, const std::string &strUserID) : m_iType(iType), m_iID(iID), m_iMsgType(iMsgType), m_strUUID(strUserID)
{

}

bool CMsgEditDB::DoWriteToJsonNode(Json::Value &rootnode)
{
	JSON_SetNodeType(rootnode, m_iMsgType);
	JSON_SetAttributeValue(rootnode, m_iType, "m_iType");
	JSON_SetAttributeValue(rootnode, m_iID, "m_iID");
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

//CMsgFileBase
bool CMsgFileBase::DoWriteToJsonNode(Json::Value &rootnode)
{
	JSON_SetNodeType(rootnode, m_iMsgType);
	JSON_SetAttributeValue(rootnode, m_strFile, "m_strFile");
	JSON_SetAttributeValue(rootnode, m_strUUID, "m_strUUID");
	return true;
}
bool CMsgFileBase::DoReadFromJsonNode(const Json::Value &rootnode)
{
	m_iMsgType = JSON_GetNodeType(rootnode);
	bool bFlag=	JSON_GetAttributeValue(rootnode, m_strFile, "m_strFile")
		&& JSON_GetAttributeValue(rootnode, m_strUUID, "m_strUUID");
	return bFlag;
}
//CMsgEditFile
CMsgEditFile::CMsgEditFile(int iMsgType, const std::string &strFile, const std::string &strUserID)
{
	 m_strFile = (strFile);
	 m_iMsgType = (iMsgType);
	 m_strUUID = (strUserID);
}


//CMsgEditOption
CMsgEditOption::CMsgEditOption(int iOptionType, const std::string &strUserID)
{
	m_strFile = GetGUID(GGT_OPTION_FILE);
	m_iMsgType = (GMT_C_EDIT_OPTION_FILE);
	m_strUUID = strUserID;
}