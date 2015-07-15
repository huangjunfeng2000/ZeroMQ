//
//  Majordomo Protocol client example
//  Uses the mdcli API to hide all MDP aspects
//
//  Lets us 'build mdclient' and 'build all'
//
//     Andreas Hoelzlwimmer <andreas.hoelzlwimmer@fh-hagenberg.at>
//
#include "mdcliapi.hpp"
#include "jsonAPI.h"
#include "log4cplus.h"
#include "msgdefine.h"

class CGDSClient
{
public:
	CGDSClient( int verbose = 1)
	{
		m_iVerbose = verbose;
		m_pMdcli = NULL;
	}
	~CGDSClient()
	{
		delete m_pMdcli;
	}
	bool ConnectToServer(const std::string &broker = "tcp://localhost:5555", 
		const std::string &strUser = "Admin", const std::string &strPassWord = "GDS")
	{
		printf("ConnectToServer...\n");
		if (m_pMdcli)
			delete m_pMdcli;
		m_strBroker = broker;
		m_pMdcli = new mdcli(m_strBroker, m_iVerbose);
		zmsg * request = new zmsg(CMsgConnectInfo(strUser, strPassWord).ToString().c_str());
		bool bSuccess = false;
		zmsg * reply = m_pMdcli->send (MSG_VIRTUAL_SERVICE, request);
		if (reply)
		{
			CMsgResult msg;
			if (msg.PraseString(reply->body()))
			{
				if (msg.m_iMsgType == GMT_S_SUCCESS)
				{
					LOG_INFO("Connect server success!");
					bSuccess = true;
				}
				else
				{
					LOG_INFO(msg.m_strInfo);
				}
			}
			else
			{
				LOG_INFO("Connect server failure!");
			}
		}
		else
		{
			LOG_INFO("Server not response!");
		}
		delete reply;
		return bSuccess;
	}
	bool GetProjectList()
	{
		printf("GetProjectList...\n");
		zmsg * request = new zmsg(CMsgGeneralCommand(GMT_SERVER_INFO).ToString().c_str());
		bool bSuccess = false;
		zmsg * reply = m_pMdcli->send (MSG_VIRTUAL_SERVICE, request);
		if (reply)
		{
			std::string strInfo = reply->body();
			Json::Value rootNode;
			if (JSON_Prase(strInfo, rootNode))
			{
				if (GMT_SERVER_INFO == JSON_GetNodeType(rootNode))
				{
					if (projList.ReadFromJsonNode(rootNode))
					{
						for (size_t i=0; i<projList.m_vecProInfo.size(); ++i)
						{
							LOG_INFO(projList.m_vecProInfo[i].ToString());
						}
						bSuccess = TRUE;
					}
				}
			}
		}
		else
		{
			LOG_INFO("Server not response!");
		}
		delete reply;
		return bSuccess;
	}
	bool OpenProject()
	{
		printf("OpenProject...\n");
		int iOpenIndex = 0;
		if (projList.m_vecProInfo.size() <= iOpenIndex)
			return false;
		CMsgProjectInfo openInfo = projList.m_vecProInfo[iOpenIndex];
		CMsgUUID MsgOpenProject(GMT_OPEN_PROJECT, openInfo.m_strUUID);
		zmsg * request = new zmsg(MsgOpenProject.ToString().c_str());
		bool bSuccess = false;
		zmsg * reply = m_pMdcli->send (MSG_VIRTUAL_SERVICE, request);
		if (reply)
		{
			std::string strInfo = reply->body();
			CMsgResult msg;
			if (msg.PraseString(reply->body()))
			{
				if (msg.m_iMsgType == GMT_S_SUCCESS)
				{
					LOG_INFO("Open project success!");
					bSuccess = true;
				}
				else
				{
					LOG_INFO(msg.m_strInfo);
				}
			}
		}
		else
		{
			LOG_INFO("Server not response!");
		}
		delete reply;
		return bSuccess;
	}


	bool LockData()
	{
		printf("LockData...\n");
		CMsgEditDB editDB(GMT_C_EDIT_DB, 1, 2);

		if (projList.m_vecProInfo.size() <= 0)
			return false;
		CMsgProjectInfo &openInfo = projList.m_vecProInfo[0];
		editDB.m_strUUID = openInfo.m_strUUID;
		zmsg * request = new zmsg(editDB.ToString(GOT_LOCK).c_str());
		bool bSuccess = false;
		std::string strServiceName = openInfo.ServiecName() + "." + openInfo.m_strUUID;
		zmsg * reply = m_pMdcli->send (strServiceName.c_str(), request);
		if (reply)
		{
			std::string strInfo = reply->body();
			CMsgResult msg;
			if (msg.PraseString(reply->body()))
			{
				if (msg.m_iMsgType == GMT_S_SUCCESS)
				{
					LOG_INFO("Lock data success!");
					bSuccess = true;
				}
				else
				{
					LOG_INFO(msg.m_strInfo);
				}
			}
		}
		else
		{
			LOG_INFO("Server not response!");
		}
		delete reply;
		return bSuccess;
	}
	bool TestFunc()
	{
		std::string strServiceName = "Map." + newGUID();
		strServiceName = strServiceName.length() > 16 ? strServiceName.substr(0, strServiceName.find_first_of(".")) : strServiceName;
		return true;
	}
private:
	std::string m_strBroker;
	int m_iVerbose;
	mdcli *m_pMdcli;
	CMsgProjectList projList;
};

int main (int argc, char *argv [])
{
	InitLog4Plus();
	//Json::Value node = JSON_GetRootNode();
	//JSON_SetAttributeValue(node, 1, "int");
	//JSON_SetAttributeValue(node, "str", "string");
	//std::string str = JSON_ToString(node);

	int verbose = (argc > 1 && strcmp (argv [1], "-v") == 0);
	CGDSClient gdsClient;
	gdsClient.TestFunc();
	bool bSuccess = gdsClient.ConnectToServer();
	bSuccess = gdsClient.GetProjectList() && bSuccess;
	bSuccess = gdsClient.OpenProject() && bSuccess;
	bSuccess = gdsClient.LockData() && bSuccess;
	//mdcli session ("tcp://localhost:5555", Sverbose);

	//int64_t heartbeat_at = s_clock ();
	//int count;
	//for (count = 0; count < 1000; count++) {
	//	zmsg * request = new zmsg("Hello world");
	//	zmsg * reply = session.send ("echo", request);
	//	if (reply) {
	//		delete reply;
	//	} else {
	//		break;              //  Interrupt or failure
	//	}
	//	s_sleep(1000);
	//}
	//double timeVal = (s_clock() - heartbeat_at) / 1000.0;
	//std::cout << count << " requests/replies processed : " << timeVal << " second" << std::endl;
	printf("Press any key to continue...");
	//getchar();
	return 0;
}
