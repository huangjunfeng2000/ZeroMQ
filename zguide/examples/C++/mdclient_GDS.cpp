//
//  Majordomo Protocol client example
//  Uses the mdcli API to hide all MDP aspects
//
//  Lets us 'build mdclient' and 'build all'
//
//     Andreas Hoelzlwimmer <andreas.hoelzlwimmer@fh-hagenberg.at>
//
#include "mdcliapi_GDS.hpp"
#include "jsonAPI.h"
#include "log4cplus.h"
#include "msgdefine.h"
#include "dbmsgmanager.h"
#include <pthread.h>

#include <list>

int verbose = false;
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
		m_pMdcli = new GDS::mdcli(m_strBroker, m_iVerbose);
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
		size_t iOpenIndex = 0;
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
		CMsgEditDB editDB(GMT_C_EDIT_DB, 1, 2, "A");

		if (projList.m_vecProInfo.size() <= 0)
			return false;
		CMsgProjectInfo &openInfo = projList.m_vecProInfo[0];
		editDB.m_strUUID = openInfo.m_strUUID;

		bool bSuccess = false;
		std::string strServiceName = openInfo.ServiecName() + "." + openInfo.m_strUUID;
		clock_t ibegin = clock();
		int iCount = 10;
		for (int i=0; i<iCount; ++i)
		{
			editDB.m_strUUID = i % 3==0 ? "A" : "B";
			zmsg * request = new zmsg(editDB.ToString(GOT_LOCK).c_str());
			zmsg * reply = m_pMdcli->send (strServiceName.c_str(), request);
			if (reply)
			{
				std::string strInfo = reply->body();
				CMsgResult msg;
				if (msg.PraseString(reply->body()))
				{
					if (msg.m_iMsgType == GMT_S_SUCCESS)
					{
						if (m_iVerbose)
							s_console("%s : Lock data success!", editDB.m_strUUID.c_str());
						bSuccess = true;
					}
					else
					{
						s_console("Failure: %s", msg.m_strInfo.c_str());
					}
				}
			}
			else
			{
				LOG_INFO("Server not response!");
			}
			delete reply;
		}
		clock_t iend = clock();
		clock_t idis = iend - ibegin;
		s_console("Spends %d second", idis);
		return bSuccess;
	}
	bool TestFunc()
	{
		std::string strServiceName = "Map." + newGUID();
		strServiceName = strServiceName.length() > 16 ? strServiceName.substr(0, strServiceName.find_first_of(".")) : strServiceName;
		

		CMsgEditDB editDB1(GMT_C_EDIT_DB, 1, 1);
		editDB1.m_iOperatorType = GOT_LOCK;
		ASSERT_C(true == m_DBManager.Process(editDB1));
		ASSERT_C(true == m_DBManager.Process(editDB1));

		CMsgEditDB editDB2(GMT_C_EDIT_DB, 1, 2);
		editDB2.m_iOperatorType = GOT_LOCK;
		ASSERT_C(true == m_DBManager.Process(editDB2));
		ASSERT_C(true == m_DBManager.Process(editDB2));

		CMsgEditDB editDB3(GMT_C_EDIT_DB, 1, 1);
		editDB3.m_iOperatorType = GOT_LOCK;
		ASSERT_C(true == m_DBManager.Process( editDB3));

		CMsgEditDB editDB4(GMT_C_EDIT_DB, 1, 1, "B");
		editDB4.m_iOperatorType = GOT_LOCK;
		ASSERT_C(false == m_DBManager.Process( editDB4));

		//unlock
		editDB1.m_iOperatorType = GOT_UNLOCK;
		ASSERT_C(true == m_DBManager.Process( editDB1));
		ASSERT_C(true == m_DBManager.Process(editDB4));
		//editDB4.m_iOperatorType = GOT_UNLOCK;
		ASSERT_C(true == m_DBManager.Process(editDB4));

		editDB2.m_iOperatorType = GOT_UNLOCK;
		ASSERT_C(true == m_DBManager.Process(editDB2));

		CMsgAddDB editAdd(1, "A");
		editAdd.m_iOperatorType = GOT_LOCK;
		ASSERT_C(true == m_DBManager.Process( editAdd));
		ASSERT_C(true == m_DBManager.Process( editAdd));

		CMsgAddDB editAdd2(1, "B");
		editAdd2.m_iOperatorType = GOT_LOCK;
		ASSERT_C(false == m_DBManager.Process( editAdd2));

		editAdd.m_iOperatorType = GOT_UNLOCK;
		ASSERT_C(true == m_DBManager.Process( editAdd));

		ASSERT_C(true == m_DBManager.Process( editAdd2));
		editAdd2.m_iOperatorType = GOT_UNLOCK;
		ASSERT_C(true == m_DBManager.Process( editAdd2));


		//Test File
		CMsgEditFile editFile1(GMT_C_EDIT_FILE, "File1", "A");
		editFile1.m_iOperatorType = GOT_LOCK;
		ASSERT_C(true == m_DBManager.Process( editFile1));
		ASSERT_C(true == m_DBManager.Process( editFile1));

		CMsgEditFile editFile2(GMT_C_EDIT_FILE, "File1", "B");
		editFile2.m_iOperatorType = GOT_LOCK;
		ASSERT_C(false == m_DBManager.Process( editFile2));

		editFile1.m_iOperatorType = GOT_UNLOCK;
		ASSERT_C(true == m_DBManager.Process( editFile1));
		ASSERT_C(true == m_DBManager.Process( editFile2));

		editFile2.m_iOperatorType = GOT_UNLOCK;
		ASSERT_C(true == m_DBManager.Process( editFile2));

		CMsgEditFile editFile3(GMT_C_READ_FILE, "File1", "A");
		editFile3.m_iOperatorType = GOT_LOCK;
		ASSERT_C(true == m_DBManager.Process( editFile3));

		CMsgEditFile editFile4(GMT_C_READ_FILE, "File1", "B");
		editFile4.m_iOperatorType = GOT_LOCK;
		ASSERT_C(true == m_DBManager.Process( editFile4));


		int iCount = 20;
		clock_t ibegin = clock();
		for (int i=0; i<iCount; ++i)
		{
			int iType = rand() % 5;
			CMsgEditDB editDB(GMT_C_EDIT_DB, iType, i);
			editDB.m_iOperatorType = GOT_LOCK;
			m_DBManager.Process(editDB);
			//CMsgEditDB editDB2(GMT_C_EDIT_DB, iType, i);
			//editDB2.m_iOperatorType = GOT_UNLOCK;
			//m_DBManager.Process(editDB2);
		}
		CMsgUUID msgUUID(GMT_C_CLOSE_PROJECT_DB, "A");
		m_DBManager.Process(msgUUID);
		msgUUID.m_iMsgType = GMT_C_CLOSE_ALL_DB;
		m_DBManager.Process(msgUUID);

		double timeVal = (clock() - ibegin) / 1000.0;
		printf("Spend time: %.3f", timeVal);
		return true;
	}
private:
	std::string m_strBroker;
	int m_iVerbose;
	GDS::mdcli *m_pMdcli;
	CMsgProjectList projList;
	CProjectMsgManager m_DBManager;
};

static void *
	subscriber (void *args) {
		zmq::context_t context(1);

		// Subscribe to everything
		zmq::socket_t subscriber(context, ZMQ_SUB);
		subscriber.connect("tcp://localhost:5556");
		subscriber.setsockopt (ZMQ_SUBSCRIBE, "", 0);
		int m_timeout = 2500;
		std::stringstream ss;

		// Get and process messages
		while (true) {
			//  Poll socket for a reply, with timeout
			zmq::pollitem_t items [] = {
				{ subscriber, 0, ZMQ_POLLIN, 0 }
			};
			zmq::poll (items, 1, m_timeout);

			if (items [0].revents & ZMQ_POLLIN) 
			{
				zmsg * recv_msg2 = new zmsg(subscriber);
				if (true) {
					s_console ("I: received publish: %s", recv_msg2->body());
					//recv_msg2->dump ();
				}
			}
		}
		return (NULL);
}


int main (int argc, char *argv [])
{
	InitLog4Plus();
	//Json::Value node = JSON_GetRootNode();
	//JSON_SetAttributeValue(node, 1, "int");
	//JSON_SetAttributeValue(node, "str", "string");
	//std::string str = JSON_ToString(node);


	verbose = true;//(argc > 1 && strcmp (argv [1], "-v") == 0);
	CGDSClient gdsClient(verbose);
	gdsClient.TestFunc();
	//return 0;

	pthread_t client_thread;
	pthread_create (&client_thread, NULL, subscriber, NULL);
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
	printf("Press any key to continue...\n");
	pthread_join (client_thread, NULL);
	//getchar();
	return 0;
}
