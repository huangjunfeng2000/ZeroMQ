//
//  Majordomo Protocol broker
//  A minimal implementation of http://rfc.zeromq.org/spec:7 and spec:8
//
//     Andreas Hoelzlwimmer <andreas.hoelzlwimmer@fh-hagenberg.at>
//
#include "zmsg.hpp"
#include "mdp.h"
#include "msgdefine.h"
#include "jsonAPI.h"

#include <map>
#include <vector>
#include <deque>
#include <list>
#include <set>

//  We'd normally pull these from config data

#define HEARTBEAT_LIVENESS  3       //  3-5 is reasonable
#define HEARTBEAT_INTERVAL  2500    //  msecs
#define HEARTBEAT_EXPIRY    HEARTBEAT_INTERVAL * HEARTBEAT_LIVENESS

namespace GDS
{

struct service;

//  This defines one worker, idle or active
struct worker
{
	std::string m_identity;   //  Address of worker
	service * m_service;      //  Owning service, if known
	int64_t m_expiry;         //  Expires at unless heartbeat
	CMsgProjectInfo m_projInfo;
	worker(std::string identity, service * service = 0, int64_t expiry = 0) {
		m_identity = identity;
		m_service = service;
		m_expiry = expiry;
	}
};

//  This defines a single service
struct service
{
	~service ()
	{
		//for(size_t i = 0; i < m_requests.size(); i++) {
		//	delete m_requests[i];
		//}
		for (iter it = map_requests.begin(); it != map_requests.end(); ++it)
		{
			std::deque<zmsg*> &requests = it->second;
			for(size_t i = 0; i < requests.size(); i++) {
				delete requests[i];
			}
		}

	}

	std::string m_name;             //  Service name
	//std::deque<zmsg*> m_requests;   //  List of client requests
	std::map<std::string, std::deque<zmsg*> > map_requests;//project uuid: request msg
	typedef std::map<std::string, std::deque<zmsg*> >::iterator iter;
	std::list<worker*> m_waiting;  //  List of waiting workers
	size_t m_workers;               //  How many workers we have

	service(std::string name)
	{
		m_name = name;
	}
};

//  This defines a single broker
class broker {
public:

	//  ---------------------------------------------------------------------
	//  Constructor for broker object

	broker (int verbose)
	{
		//  Initialize broker state
		m_context = new zmq::context_t(1);
		m_socket = new zmq::socket_t(*m_context, ZMQ_ROUTER);
		//m_socketpub = zmq_socket (*m_context, ZMQ_PUB);
		m_socketpub = new zmq::socket_t(*m_context, ZMQ_PUB);
		m_verbose = verbose;
		m_heartbeat_at = s_clock () + HEARTBEAT_INTERVAL;
	}

	//  ---------------------------------------------------------------------
	//  Destructor for broker object

	virtual
		~broker ()
	{
		while (! m_services.empty())
		{
			delete m_services.begin()->second;
			m_services.erase(m_services.begin());
		}
		while (! m_workers.empty())
		{
			delete m_workers.begin()->second;
			m_workers.erase(m_workers.begin());
		}
	}

	//  ---------------------------------------------------------------------
	//  Bind broker to endpoint, can call this multiple times
	//  We use a single socket for both clients and workers.

	void
		bind (std::string endpoint)
	{
		m_endpoint = endpoint;
		m_socket->bind(m_endpoint.c_str());
		m_socketpub->bind("tcp://*:5556");
		s_console ("I: MDP broker/0.1.1 is active at %s", endpoint.c_str());
	}

private:

	//  ---------------------------------------------------------------------
	//  Delete any idle workers that haven't pinged us in a while.

	void
		purge_workers ()
	{
		int64_t now = s_clock();
		for (size_t i = 0; i < m_waiting.size();)
		{
			worker* wrk = m_waiting[i];
			if (wrk->m_expiry > now)
			{
				++i;
				continue;
			}
			if (m_verbose) {
				s_console ("I: deleting expired worker: %s %d",
					wrk->m_identity.c_str(), m_waiting.size()-1);
			}
			worker_delete (wrk, 0);
		}
	}

	//  ---------------------------------------------------------------------
	//  Locate or create new service entry

	service *
		service_require (std::string name)
	{
		assert (name.size()>0);
		if (m_services.count(name)) {
			return m_services.at(name);
		} else {
			service * srv = new service(name);
			m_services.insert(std::make_pair(name, srv));
			if (m_verbose) {
				s_console ("I: received message:");
			}
			return srv;
		}
	}



	//  ---------------------------------------------------------------------
	//  Dispatch requests to waiting workers as possible

	void
		service_dispatch (service *srv, zmsg *msg, const std::string &strUuid = "")
	{
		assert (srv);
		if (msg) {                    //  Queue message if any
			//srv->m_requests.push_back(msg);
			srv->map_requests[strUuid].push_back(msg);
		}

		purge_workers ();
		bool bProcessed = true;
		while (! srv->m_waiting.empty() && ! srv->map_requests.empty() && bProcessed)
		{
			// Choose the most recently seen idle worker; others might be about to expire
			///*std::list<worker*>::iterator wrk = srv->m_waiting.begin(*/);
			//std::list<worker*>::iterator next = wrk;
			//for (++next; next != srv->m_waiting.end(); ++next)
			//{
			//	if ((*next)->m_expiry > (*wrk)->m_expiry)
			//		wrk = next;
			//}
			//zmsg *msg = srv->m_requests.front();
			//srv->m_requests.pop_front();
			//worker_send (*wrk, (char*)MDPW_REQUEST, "", msg);
			//for (size_t i=0; i<m_waiting.size(); )
			//{
			//	if (m_waiting[i] == *wrk) 
			//	{
			//		m_waiting.erase(m_waiting.begin() + i);
			//	}
			//	else
			//	{
			//		++i;
			//	}
			//}
			//srv->m_waiting.erase(wrk);
			//delete msg;
			//检查相同工区ID
			std::set<std::string> setIDs;
			for (std::list<worker*>::iterator wrk = srv->m_waiting.begin(); wrk != srv->m_waiting.end(); ++wrk)
			{
				if (setIDs.count((*wrk)->m_projInfo.m_strUUID))
					s_console("Error: Same Project ID: %s", (*wrk)->m_projInfo.m_strUUID.c_str());
				setIDs.insert((*wrk)->m_projInfo.m_strUUID);
			}


			bProcessed = false;
			for (std::list<worker*>::iterator wrk = srv->m_waiting.begin(); !bProcessed && wrk != srv->m_waiting.end() && !srv->map_requests.empty(); ++wrk)
			{
				std::list<worker*>::iterator next = wrk;
				for (++next; next != srv->m_waiting.end(); ++next)
				{
					if ((*wrk)->m_projInfo.m_strUUID == (*next)->m_projInfo.m_strUUID)
					{
						if ((*next)->m_expiry > (*wrk)->m_expiry)
							wrk = next;
					}
				}
				service::iter it = srv->map_requests.find((*wrk)->m_projInfo.m_strUUID);
				if (it != srv->map_requests.end() && !it->second.empty())
				{
					bProcessed = true;
					zmsg *msg = it->second.front();
					it->second.pop_front();
					if (it->second.empty())
					{
						srv->map_requests.erase(it);
					}
					worker_send (*wrk, (char*)MDPW_REQUEST, "", msg);
					for (size_t i=0; i<m_waiting.size(); )
					{
						if (m_waiting[i] == *wrk) 
						{
							m_waiting.erase(m_waiting.begin() + i);
						}
						else
						{
							++i;
						}
					}
					srv->m_waiting.erase(wrk);
					delete msg;
					break;
				}
			}
	
		}
	}

	//  ---------------------------------------------------------------------
	//  Handle internal service according to 8/MMI specification

	void
		service_internal (std::string service_name, zmsg *msg)
	{
		if (service_name.compare("mmi.service") == 0) {
			service * srv = m_services.at(msg->body());
			if (srv && srv->m_workers) {
				msg->body_set("200");
			} else {
				msg->body_set("404");
			}
		} else {
			msg->body_set("501");
		}

		//  Remove & save client return envelope and insert the
		//  protocol header and service name, then rewrap envelope.
		std::string client = msg->unwrap();
		msg->wrap(MDPC_CLIENT, service_name.c_str());
		msg->wrap(client.c_str(), "");
		msg->send (*m_socket);
	}

	//  ---------------------------------------------------------------------
	//  Handle internal virtual service 

	void
		service_virtual (std::string service_name, zmsg *msg)
	{
		if (service_name.compare(MSG_VIRTUAL_SERVICE) == 0) 
		{
			std::string strBody = msg->body();
			Json::Value rootNode;
			if (JSON_Prase(strBody, rootNode))
			{
				int iType = JSON_GetNodeType(rootNode);
				switch(iType)
				{
				case GMT_CONNECT_SERVER:
					{
						CMsgConnectInfo connectInfo;
						connectInfo.ReadFromJsonNode(rootNode);
						s_console("User: %s Pass: %s", connectInfo.m_strUser.c_str(), connectInfo.m_strPassWord.c_str());
						CMsgResult msgRes(GMT_S_SUCCESS, "");
						msg->body_set(msgRes.ToString().c_str());
					}
					break;
				case GMT_SERVER_INFO:
					{
						//typedef std::map<std::string, service*>::iterator iter;
						typedef std::map<std::string, worker*>::iterator iter;
						CMsgProjectList projLst;
						for (iter it = m_workers.begin(); it != m_workers.end(); ++it)
						{
							projLst.m_vecProInfo.push_back(it->second->m_projInfo);
						}
						msg->body_set(projLst.ToString().c_str());
					}
					break;
				case GMT_OPEN_PROJECT:
					{
						CMsgUUID msgInfo;
						msgInfo.ReadFromJsonNode(rootNode);
						s_console("Open Project UUID: %s", msgInfo.m_strUUID.c_str());
						std::string strErrorInfo = "No Project";
						int iMsgType = GMT_S_FAILURE;
						typedef std::map<std::string, worker*>::iterator iter;
						for (iter it = m_workers.begin(); it != m_workers.end(); ++it)
						{
							if (it->second->m_projInfo.m_strUUID == msgInfo.m_strUUID)
							iMsgType = GMT_S_SUCCESS;
						}
						CMsgResult msgRes(iMsgType, strErrorInfo);
						msg->body_set(msgRes.ToString().c_str());
					}
					break;
				default:
					msg->body_set(MSG_FAILURE);
				}
			}
		} 
		else 
		{
			msg->body_set(MSG_FAILURE);
		}

		//  Remove & save client return envelope and insert the
		//  protocol header and service name, then rewrap envelope.
		std::string client = msg->unwrap();
		msg->wrap(MDPC_CLIENT, service_name.c_str());
		msg->wrap(client.c_str(), "");
		msg->send (*m_socket);
	}
	//  ---------------------------------------------------------------------
	//  Creates worker if necessary

	worker *
		worker_require (std::string identity)
	{
		assert (identity.length()!=0);

		//  self->workers is keyed off worker identity
		if (m_workers.count(identity)) {
			return m_workers.at(identity);
		} else {
			worker *wrk = new worker(identity);
			m_workers.insert(std::make_pair(identity, wrk));
			if (m_verbose) {
				s_console ("I: registering new worker: %s", identity.c_str());
			}
			return wrk;
		}
	}

	//  ---------------------------------------------------------------------
	//  Deletes worker from all data structures, and destroys worker

	void
		worker_delete (worker *&wrk, int disconnect)
	{
		assert (wrk);
		if (disconnect) {
			worker_send (wrk, (char*)MDPW_DISCONNECT, "", NULL);
		}

		if (wrk->m_service) {
			for(std::list<worker*>::iterator it = wrk->m_service->m_waiting.begin();
				it != wrk->m_service->m_waiting.end();) {
					if (*it == wrk) {
						it = wrk->m_service->m_waiting.erase(it);
					}
					else {
						++it;
					}
			}
			wrk->m_service->m_workers--;
		}
		for (size_t i=0; i<m_waiting.size(); )
		{
			if (m_waiting[i] == wrk) 
			{
				m_waiting.erase(m_waiting.begin() + i);
			}
			else
			{
				++i;
			}
		}
		//   for(std::vector<worker*>::iterator it = m_waiting.begin(); it != m_waiting.end(); it++) {
		//      if (*it == wrk) {
		//         it = m_waiting.erase(it);
		//if (it == m_waiting.end())
		// break;
		//--it;
		//      }
		//   }
		//  This implicitly calls the worker destructor
		CMsgUUID msgUUID(GMT_W_CLOSED, wrk->m_identity);
		client_pub(msgUUID.ToString());

		m_workers.erase(wrk->m_identity);
		delete wrk;
	}

	void client_pub(const std::string &str)
	{
		s_send (*m_socketpub, str.c_str());
	}

	//  ---------------------------------------------------------------------
	//  Process message sent to us by a worker

	void
		worker_process (std::string sender, zmsg *msg)
	{
		assert (msg && msg->parts() >= 1);     //  At least, command

		std::string command = (char *)msg->pop_front().c_str();
		bool worker_ready = m_workers.count(sender)>0;
		worker *wrk = worker_require (sender);

		if (command.compare (MDPW_READY) == 0) {
			if (worker_ready)  {              //  Not first command in session
				worker_delete (wrk, 1);
			}
			else {
				if (sender.size() >= 4  //  Reserved service name
					&&  sender.find_first_of("mmi.") == 0) {
						worker_delete (wrk, 1);
				} else {
					//  Attach worker to service and mark as idle
					std::string service_name = (char*)msg->pop_front ().c_str();
					std::string strBody = msg->body();
					Json::Value rootNode;
					if (JSON_Prase(strBody, rootNode))
					{
						wrk->m_projInfo.ReadFromJsonNode(rootNode);
					}

					wrk->m_service = service_require (service_name);
					wrk->m_service->m_workers++;
					worker_waiting (wrk);
				}
			}
		} else {
			if (command.compare (MDPW_REPLY) == 0) {
				if (worker_ready) {
					//  Remove & save client return envelope and insert the
					//  protocol header and service name, then rewrap envelope.
					std::string client = msg->unwrap ();
					msg->wrap (MDPC_CLIENT, wrk->m_service->m_name.c_str());
					msg->wrap (client.c_str(), "");
					msg->send (*m_socket);
					worker_waiting (wrk);
				}
				else {
					worker_delete (wrk, 1);
				}
			} 
			else if (command.compare (MDPW_PUBLISH) == 0) 
			{
					//  Remove & save client return envelope and insert the
					//  protocol header and service name, then rewrap envelope.
					//std::string client = msg->unwrap ();
					//msg->wrap (MDPC_CLIENT, wrk->m_service->m_name.c_str());
					//msg->wrap (client.c_str(), "");
					//msg->send(*m_socketpub);
				std::string body = msg->body ();
					//s_send (*m_socketpub, msg->body());
				client_pub(body);
					//s_send (m_socketpub, client);
			} 
			else 
			{
				if (command.compare (MDPW_HEARTBEAT) == 0) {
					//s_console ("E: received message work heartbeats");
					if (worker_ready) {
						wrk->m_expiry = s_clock () + HEARTBEAT_EXPIRY;
					} else {
						worker_delete (wrk, 1);
					}
				} else {
					if (command.compare (MDPW_DISCONNECT) == 0) {
						worker_delete (wrk, 0);
					} else {
						s_console ("E: invalid input message (%d)", (int) *command.c_str());
						msg->dump ();
					}
				}
			}
		}
		delete msg;
	}

	//  ---------------------------------------------------------------------
	//  Send message to worker
	//  If pointer to message is provided, sends that message

	void
		worker_send (worker *worker,
		char *command, std::string option, zmsg *msg)
	{
		msg = (msg ? new zmsg(*msg) : new zmsg ());

		//  Stack protocol envelope to start of message
		if (option.size()>0) {                 //  Optional frame after command
			msg->push_front ((char*)option.c_str());
		}
		msg->push_front (command);
		msg->push_front ((char*)MDPW_WORKER);
		//  Stack routing envelope to start of message
		msg->wrap(worker->m_identity.c_str(), "");

		if (m_verbose) {
			s_console ("I: sending %s to worker",
				mdps_commands [(int) *command]);
			msg->dump ();
		}
		msg->send (*m_socket);
		delete msg;
	}

	//  ---------------------------------------------------------------------
	//  This worker is now waiting for work

	void
		worker_waiting (worker *worker)
	{
		assert (worker);
		//  Queue to broker and service waiting lists
		m_waiting.push_back(worker);
		worker->m_service->m_waiting.push_back(worker);
		worker->m_expiry = s_clock () + HEARTBEAT_EXPIRY;
		// Attempt to process outstanding requests
		service_dispatch (worker->m_service, 0);
	}



	//  ---------------------------------------------------------------------
	//  Process a request coming from a client

	void
		client_process (std::string sender, zmsg *msg)
	{
		assert (msg && msg->parts () >= 2);     //  Service name + body

		std::string service_body = msg->body();
		std::string service_name = (char *)msg->pop_front().c_str();
		std::string service_uuid;
		if (service_name.length() > 32)
		{
			service_uuid =  service_name.substr(service_name.find_first_of(".")+1);
			service_name =  service_name.substr(0, service_name.find_first_of("."));
		}

		service *srv = service_require (service_name);
		//  Set reply return address to client sender
		msg->wrap (sender.c_str(), "");
		if (service_name.compare(MSG_VIRTUAL_SERVICE) == 0)
		{
			service_virtual(service_name, msg);
		}
		else if (service_name.length() >= 4 &&  service_name.find_first_of("mmi.") == 0) 
		{
			service_internal (service_name, msg);
		} 
		else 
		{
			service_dispatch (srv, msg, service_uuid);
		}
	}

public:

	//  Get and process messages forever or until interrupted
	void
		start_brokering() {
			while (!s_interrupted) {
				zmq::pollitem_t items [] = {
					{ *m_socket,  0, ZMQ_POLLIN, 0 } };
					zmq::poll (items, 1, HEARTBEAT_INTERVAL);

					//  Process next input message, if any
					if (items [0].revents & ZMQ_POLLIN) {
						zmsg *msg = new zmsg(*m_socket);
						if (m_verbose) {
							s_console ("I: received message:");
							msg->dump ();
						}
						std::string sender = std::string((char*)msg->pop_front ().c_str());
						msg->pop_front (); //empty message
						std::string header = std::string((char*)msg->pop_front ().c_str());

						//              std::cout << "sbrok, sender: "<< sender << std::endl;
						//              std::cout << "sbrok, header: "<< header << std::endl;
						//              std::cout << "msg size: " << msg->parts() << std::endl;
						//              msg->dump();
						if (header.compare(MDPC_CLIENT) == 0) {
							client_process (sender, msg);
						}
						else if (header.compare(MDPW_WORKER) == 0) {
							worker_process (sender, msg);
						}
						else {
							s_console ("E: invalid message:");
							msg->dump ();
							delete msg;
						}
					}
					//  Disconnect and delete any expired workers
					//  Send heartbeats to idle workers if needed
					int64_t sclock = s_clock();
					if (sclock > m_heartbeat_at) {
						purge_workers ();
						for (std::vector<worker*>::iterator it = m_waiting.begin();
							it != m_waiting.end() && (*it)!=0; it++) {
								worker_send (*it, (char*)MDPW_HEARTBEAT, "", NULL);
						}
						m_heartbeat_at = s_clock () + HEARTBEAT_INTERVAL;
					}
			}
	}

private:
	zmq::context_t * m_context;                  //  0MQ context
	zmq::socket_t * m_socket;                    //  Socket for clients & workers
	zmq::socket_t * m_socketpub;                 //  Socket publish for clients & workers
	int m_verbose;                               //  Print activity to stdout
	std::string m_endpoint;                      //  Broker binds to this endpoint
	std::map<std::string, service*> m_services;  //  Hash of known services
	std::map<std::string, worker*> m_workers;    //  Hash of known workers
	std::vector<worker*> m_waiting;              //  List of waiting workers
	int64_t m_heartbeat_at;                     //  When to send HEARTBEAT
};

bool CreateWorker(const std::string &strProject)
{
	STARTUPINFO si; //一些必备参数设置  
	memset(&si, 0, sizeof(STARTUPINFO));  
	si.cb = sizeof(STARTUPINFO);  
	si.dwFlags = STARTF_USEPOSITION;//STARTF_USESHOWWINDOW ;  
	si.wShowWindow = SW_SHOWNORMAL;  
	si.dwX = 600;
	si.dwY = 0;
	WORD createFlag = CREATE_NEW_CONSOLE;
	createFlag = NULL;
	PROCESS_INFORMATION pi; //必备参数设置结束  
	if(!CreateProcess(NULL,"cpp.worker.exe -v",NULL,NULL,FALSE,createFlag,NULL,NULL,&si,&pi)) //8888为命令行参数，ExcuteApp.exe为当前目录下的一个exe文件。  
	{  
		std::cout<<"Create Fail!"<<std::endl;  
		return false;  
	}  
	else  
	{
		std::cout<<"CreateWorker Sucess!"<<std::endl;
	}  
	return true;  

}
//  ---------------------------------------------------------------------
//  Main broker work happens here
};

int main (int argc, char *argv [])
{
	int verbose = (argc > 1 && strcmp (argv [1], "-v") == 0);

	//CreateWorker("c:\\1.uni");
	s_version_assert (4, 0);
	s_catch_signals ();
	GDS::broker brk(verbose);
	brk.bind ("tcp://*:5555");

	brk.start_brokering();

	if (s_interrupted)
		printf ("W: interrupt received, shutting down...\n");

	return 0;
}


