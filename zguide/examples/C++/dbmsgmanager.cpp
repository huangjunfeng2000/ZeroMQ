#include "dbmsgmanager.h"

class CLockDataMsgDB;
class CUnLockDataMsgDB;
class CCheckValidMsgDB;

//CMsgProcessBase
CMsgProcessBase::CMsgProcessBase() : m_pNext(NULL), m_iMask(0), pMsg(NULL), iMsgType(0)
{}
CMsgProcessBase::~CMsgProcessBase() 
{}

bool CMsgProcessBase::Process(size_t iMsgTypeValue, const CMsgBase *pMsgValue)
{
	iMsgType = iMsgTypeValue;
	pMsg = pMsgValue;
	if (CanProcess())
		return DoProcess();
	else
		return ProcessNext();
}
bool CMsgProcessBase::ProcessDispatch()
{
	switch(pMsg->m_iOperatorType)
	{
	case GOT_LOCK:
		return DoProcessLock();
	case GOT_UNLOCK:
		return DoProcessUnLock();
	default:
		return ProcessNext();
	}
}
bool CMsgProcessBase::DoProcessLock()
{
	return ProcessNext();
}
bool CMsgProcessBase::DoProcessUnLock()
{
	return ProcessNext();
}
bool CMsgProcessBase::Process(const CMsgBase &msgValue)
{
	return Process(msgValue.Type(), &msgValue);
}
bool CMsgProcessBase::DoProcess()
{
	return ProcessDispatch();
}

bool CMsgProcessBase::ProcessNext()
{
	//return m_pNext ? m_pNext->Process(iMsgType, pMsg) : false;
	if (m_pNext)
		return m_pNext->Process(iMsgType, pMsg);
	//s_console("Not process Msg: Type(%d) Value(%s)", iMsgType, pMsg->ToString());
	return true;
}
bool CMsgProcessBase::CanProcess()
{
	return (iMsgType & m_iMask) != 0;
}
//bool CMsgProcessBase::IsStateConflict(size_t scr, size_t des)	//check state conflict true: conflict
//{
//	return true;
//}
//CMsgProcessBaseDB
CMsgProcessBaseDB::CMsgProcessBaseDB() : m_pManager(NULL)
{

}
//bool CMsgProcessBaseDB::IsStateConflict(size_t scr, size_t des)	//check state conflict true: conflict
//{
//	if ((scr & des) == GMT_C_EDIT_DB) return true;
//	if ((scr & des) == GMT_C_DELETE_DB) return true;
//	if ((scr & des) == GMT_C_ADD_DB) return true;
//	ASSERT_C((scr & des) != GMT_C_DELETE_DB);
//	return false;
//}
//CMsgProcessBaseFile
CMsgProcessBaseFile::CMsgProcessBaseFile() : m_pManager(NULL)
{

}
//bool CMsgProcessBaseFile::IsStateConflict(size_t scr, size_t des)	//check state conflict true: conflict
//{
//	if ((scr & des) == GMT_C_EDIT_FILE) return true;
//	if ((scr & des) == GMT_C_READ_FILE && scr==des) return false;
//	return false;
//}

//CCheckValidMsgDB
class CCheckValidMsgDB : public CMsgProcessBaseDB
{
public:
	CCheckValidMsgDB(){}
	virtual bool DoProcess()
	{
		if (!(iMsgType & GMT_C_CLOSE_PROJECT_DB || iMsgType & GMT_C_CLOSE_ALL_DB || iMsgType & GMT_C_ADD_DB))
		{
			CMsgEditDB *pData = (CMsgEditDB *)(pMsg);
			if (pData->m_iID < 0)	//新增数据不处理
				return true;
		}
		return ProcessNext();
	}
};

//CLockDataMsgDB 加锁\解锁数据(CMsgEditDB)
class CMsgEditDBProcess : public CMsgProcessBaseDB
{
public:
	CMsgEditDBProcess(){}
	//virtual bool CanProcess()
	//{
	//	return (CMsgProcessBase::CanProcess())
	//	//{
	//	//	return pMsg->m_iOperatorType == GOT_LOCK;
	//	//}
	//	//return false;
	//}
	//virtual bool DoProcess()
	//{
	//	if (pMsg->m_iOperatorType == GOT_LOCK)
	//		return DoProcessLock();
	//	else if (pMsg->m_iOperatorType == GOT_LOCK)
	//		return DoProcessUnLock();
	//	ASSERT_C(false);
	//	return ProcessNext();
	//}
	bool DoProcessLock()
	{
		const CMsgEditDB &editInfo = *(const CMsgEditDB *)pMsg;
		std::map<CDBItemInfo, std::map<int, size_t> > &map_UserStates = m_pManager->map_UserStates;
		typedef std::map<CDBItemInfo, std::map<int, size_t> >::iterator iter;
		typedef std::map<int, size_t>::iterator iterUser;
		CDBItemInfo item(editInfo.m_iType, editInfo.m_iID);
		iter it = map_UserStates.find(item);
		int iIdIndex = m_pManager->GetUserIndex(editInfo.m_strUUID);
		if (it != map_UserStates.end())	//find type id
		{
			iterUser itUserFind = it->second.find(iIdIndex);
			if (itUserFind != it->second.end())	//find user
			{
				if (itUserFind->second & editInfo.m_iMsgType)
					return true;	//include state 
			}

			//check conflict
			{
				iterUser itUser = it->second.begin();
				for (; itUser != it->second.end(); ++itUser)
				{
					if (IsStateConflict(itUser->second, editInfo.m_iMsgType) && (itUserFind == it->second.end() || itUserFind != itUser))
					{
						return false;
					}
				}
			}
			//find user
			if (itUserFind != it->second.end())	
			{
				itUserFind->second = itUserFind->second | editInfo.m_iMsgType;
			}
			else
			{
				it->second[iIdIndex] = editInfo.m_iMsgType;
			}
		}
		else	//not have id type
		{
			map_UserStates[item][iIdIndex] = editInfo.m_iMsgType;
		}
		return true;
	}
	virtual bool DoProcessUnLock()
	{
		const CMsgEditDB &editInfo = *(const CMsgEditDB *)pMsg;
		std::map<CDBItemInfo, std::map<int, size_t> > &map_UserStates = m_pManager->map_UserStates;
		typedef std::map<CDBItemInfo, std::map<int, size_t> >::iterator iter;
		typedef std::map<int, size_t>::iterator iterUser;
		CDBItemInfo item(editInfo.m_iType, editInfo.m_iID);
		iter it = map_UserStates.find(item);
		int iIdIndex = m_pManager->GetUserIndex(editInfo.m_strUUID);
		if (it != map_UserStates.end())	//find type id
		{
			iterUser itUserFind = it->second.find(iIdIndex);
			if (itUserFind != it->second.end())	//find user
			{
				ASSERT_C (itUserFind->second & editInfo.m_iMsgType);
				itUserFind->second = itUserFind->second & (~editInfo.m_iMsgType);
				if (itUserFind->second == 0)
				{
					it->second.erase(itUserFind);
					if (it->second.empty())
					{
						map_UserStates.erase(it);
					}
				}
				return true;
			}
		}
		ASSERT_C(false);
		return true;	//unlock invalid state 
	}
	bool IsStateConflict(size_t scr, size_t des)	//check state conflict true: conflict
	{
		if ((scr & des) == GMT_C_EDIT_DB) return true;
		if ((scr & des) == GMT_C_DELETE_DB) return true;
		//if ((scr & des) == GMT_C_ADD_DB) return true;
		ASSERT_C((scr & des) != GMT_C_DELETE_DB);
		return false;
	}
};
//CLockDataMsgDB 加锁数据 CMsgAddDB
class CMsgAddDBProcess : public CMsgProcessBaseDB
{
public:
	CMsgAddDBProcess() {}
	bool DoProcessLock()
	{
		const CMsgAddDB &editInfo = *(const CMsgAddDB *)pMsg;
		std::map<int, std::map<int, size_t> > &map_TableStates = m_pManager->map_TableStates;
		typedef std::map<int, std::map<int, size_t> >::iterator iter;
		typedef std::map<int, size_t>::iterator iterUser;
		//CDBItemInfo item(editInfo.m_iType, editInfo.m_iID);
		iter it = map_TableStates.find(editInfo.m_iTableType);
		int iIdIndex = m_pManager->GetUserIndex(editInfo.m_strUUID);
		if (it != map_TableStates.end())	//find type id
		{
			iterUser itUserFind = it->second.find(iIdIndex);
			if (itUserFind != it->second.end())	//find user
			{
				if (itUserFind->second & editInfo.Type())
					return true;	//include state 
			}

			////check conflict
			{
				iterUser itUser = it->second.begin();
				for (; itUser != it->second.end(); ++itUser)
				{
					if (IsStateConflict(itUser->second, editInfo.Type()) && (itUserFind == it->second.end() || itUserFind != itUser))
					{
						return false;
					}
				}
			}
			//find user
			if (itUserFind != it->second.end())	
			{
				itUserFind->second = itUserFind->second | editInfo.Type();
			}
			else
			{
				it->second[iIdIndex] = editInfo.Type();
			}
		}
		else	//not have id type
		{
			map_TableStates[editInfo.m_iTableType][iIdIndex] = editInfo.Type();
		}
		return true;
	}
	virtual bool DoProcessUnLock()
	{
		const CMsgAddDB &editInfo = *(const CMsgAddDB *)pMsg;
		std::map<int, std::map<int, size_t> > &map_TableStates = m_pManager->map_TableStates;
		typedef std::map<int, std::map<int, size_t> >::iterator iter;
		typedef std::map<int, size_t>::iterator iterUser;
		//CDBItemInfo item(editInfo.m_iType, editInfo.m_iID);
		iter it = map_TableStates.find(editInfo.m_iTableType);
		int iIdIndex = m_pManager->GetUserIndex(editInfo.m_strUUID);
		if (it != map_TableStates.end())	//find type id
		{
			iterUser itUserFind = it->second.find(iIdIndex);
			if (itUserFind != it->second.end())	//find user
			{
				ASSERT_C (itUserFind->second & editInfo.Type());
				itUserFind->second = itUserFind->second & (~editInfo.Type());
				if (itUserFind->second == 0)
				{
					it->second.erase(itUserFind);
					if (it->second.empty())
					{
						map_TableStates.erase(it);
					}
				}
				return true;
			}
		}
		ASSERT_C(false);
		return true;	//unlock invalid state 
	}
	bool IsStateConflict(size_t scr, size_t des)	//check state conflict true: conflict
	{
		//if ((scr & des) == GMT_C_EDIT_DB) return true;
		//if ((scr & des) == GMT_C_DELETE_DB) return true;
		if ((scr & des) == GMT_C_ADD_DB) return true;
		//ASSERT_C((scr & des) != GMT_C_DELETE_DB);
		return false;
	}
};

//CCloseProjectMsgDB
class CCloseProjectMsgDB : public CMsgProcessBaseDB
{
public:
	CCloseProjectMsgDB(){}
	virtual bool DoProcess()
	{
		CMsgUUID *pData = (CMsgUUID *)(pMsg);

		int iIdIndex = m_pManager->FindUserIndex(pData->m_strUUID);
		if (iIdIndex <= 0)
			return true;

		std::map<CDBItemInfo, std::map<int, size_t> > &map_UserStates = m_pManager->map_UserStates;
		typedef std::map<CDBItemInfo, std::map<int, size_t> >::iterator iter;
		typedef std::map<int, size_t>::iterator iterUser;
		for (iter it = map_UserStates.begin(); it!=map_UserStates.end(); )
		{
			iterUser itUserFind = it->second.find(iIdIndex);
			if (itUserFind != it->second.end())	//find user
			{
				iter it_back = it;
				bool is_first_element = false;
				if(it_back != map_UserStates.begin())
					it_back --;
				else
					is_first_element = true;
				it->second.erase(itUserFind);
				if (it->second.empty())
				{
					map_UserStates.erase(it);
				}
				if(is_first_element)
					it = map_UserStates.begin();
				else
					it = ++ it_back;
			}
			else
			{
				++it;
			}
		}
		m_pManager->map_UserID.erase(pData->m_strUUID);

		return true;
	}
};

//CCloseAllMsgDB
class CCloseAllMsgDB : public CMsgProcessBaseDB
{
public:
	CCloseAllMsgDB(){}
	virtual bool DoProcess()
	{
		CMsgUUID *pData = (CMsgUUID *)(pMsg);

		m_pManager->map_UserID.clear();
		m_pManager->map_UserStates.clear();

		return true;
	}
};
CMsgManagerBase::CMsgManagerBase()
{

}
int CMsgManagerBase::GetUserIndex(const std::string &strUUID)
{
	static int s_iUserIndex = 1;
	if (int iID = FindUserIndex(strUUID))
		return iID;
	map_UserID[strUUID] = s_iUserIndex;
	return s_iUserIndex++;
}
int  CMsgManagerBase::FindUserIndex(const std::string &strUUID)
{
	typedef std::map<std::string, int>::iterator iter;
	iter it = map_UserID.find(strUUID);
	if (it != map_UserID.end())
		return it->second;
	return 0;
}

//CDBMsgManager
CDBMsgManager::CDBMsgManager()
{
	m_iMask = GMT_C_EDIT_DB | GMT_C_ADD_DB | GMT_C_DELETE_DB | GMT_C_CLOSE_PROJECT_DB;
	AddProcessor(this, new CCheckValidMsgDB(), GMT_C_EDIT_DB | GMT_C_ADD_DB | GMT_C_DELETE_DB);
	AddProcessor(this, new CMsgEditDBProcess, GMT_C_EDIT_DB | GMT_C_DELETE_DB);
	AddProcessor(this, new CMsgAddDBProcess(), GMT_C_ADD_DB);
	AddProcessor(this, new CCloseProjectMsgDB(), GMT_C_CLOSE_PROJECT_DB);
	AddProcessor(this, new CCloseAllMsgDB(), GMT_C_CLOSE_ALL_DB);
}
bool CDBMsgManager::CanProcess()
{
	if (CMsgProcessBase::CanProcess())
	{
		const CMsgEditDB &editInfo = *(const CMsgEditDB *)pMsg;
		return pMsg->m_iOperatorType == GOT_LOCK || pMsg->m_iOperatorType == GOT_UNLOCK;
	}
	return false;
}
void CDBMsgManager::AddProcessor(CDBMsgManager *pManager, CMsgProcessBaseDB *pProcessor, size_t iMask)
{
	pProcessor->m_iMask = iMask;
	//pProcessor->m_pNext = m_pNext;
	pProcessor->m_pManager = pManager;
	CMsgProcessBase *pLast = m_pNext;
	while(pLast && pLast->m_pNext)
	{
		pLast = pLast->m_pNext;
	}
	pLast ? pLast->m_pNext = pProcessor : m_pNext = pProcessor;
}

//CMsgEditFileProcess 加锁数据 CMsgEditFile
class CMsgEditFileProcess : public CMsgProcessBaseFile
{
public:
	CMsgEditFileProcess() {}
	bool DoProcessLock()
	{
		const CMsgFileBase &editInfo = *(const CMsgFileBase *)pMsg;
		std::map<int, std::map<int, size_t> > &map_UserStates = m_pManager->map_UserStates;
		typedef std::map<int, std::map<int, size_t> >::iterator iter;
		typedef std::map<int, size_t>::iterator iterUser;
		int iFileIdIndex = m_pManager->GetFileIndex(editInfo.m_strFile);
		iter it = map_UserStates.find(iFileIdIndex);
		int iIdIndex = m_pManager->GetUserIndex(editInfo.m_strUUID);
		if (it != map_UserStates.end())	//find type id
		{
			iterUser itUserFind = it->second.find(iIdIndex);
			if (itUserFind != it->second.end())	//find user
			{
				if (itUserFind->second & editInfo.m_iMsgType)
					return true;	//include state 
			}

			//check conflict
			{
				iterUser itUser = it->second.begin();
				for (; itUser != it->second.end(); ++itUser)
				{
					if (IsStateConflict(itUser->second, editInfo.m_iMsgType) && (itUserFind == it->second.end() || itUserFind != itUser))
					{
						return false;
					}
				}
			}
			//find user
			if (itUserFind != it->second.end())	
			{
				itUserFind->second = itUserFind->second | editInfo.m_iMsgType;
			}
			else
			{
				it->second[iIdIndex] = editInfo.m_iMsgType;
			}
		}
		else	//not have id type
		{
			map_UserStates[iFileIdIndex][iIdIndex] = editInfo.m_iMsgType;
		}
		return true;
	}
	virtual bool DoProcessUnLock()
	{
		const CMsgFileBase &editInfo = *(const CMsgFileBase *)pMsg;
		std::map<int, std::map<int, size_t> > &map_UserStates = m_pManager->map_UserStates;
		typedef std::map<int, std::map<int, size_t> >::iterator iter;
		typedef std::map<int, size_t>::iterator iterUser;
		//CDBItemInfo item(editInfo.m_iType, editInfo.m_iID);
		int iFileIdIndex = m_pManager->FindFileIndex(editInfo.m_strFile);
		iter it = map_UserStates.find(iFileIdIndex);
		int iIdIndex = m_pManager->GetUserIndex(editInfo.m_strUUID);
		if (it != map_UserStates.end())	//find type id
		{
			iterUser itUserFind = it->second.find(iIdIndex);
			if (itUserFind != it->second.end())	//find user
			{
				ASSERT_C (itUserFind->second & editInfo.m_iMsgType);
				itUserFind->second = itUserFind->second & (~editInfo.m_iMsgType);
				if (itUserFind->second == 0)
				{
					it->second.erase(itUserFind);
					if (it->second.empty())
					{
						map_UserStates.erase(it);
					}
				}
				return true;
			}
		}
		ASSERT_C(false);
		return true;	//unlock invalid state 
	}
	bool IsStateConflict(size_t scr, size_t des)	//check state conflict true: conflict
	{
		if ((scr & des) == GMT_C_EDIT_FILE) return true;
		if ((scr & des) == GMT_C_READ_FILE && scr==des) return false;
		return false;
	}
};
//CFileMsgManager
CFileMsgManager::CFileMsgManager()
{
	m_iMask = GMT_C_EDIT_FILE | GMT_C_READ_FILE;
	AddProcessor(this, new CMsgEditFileProcess(), GMT_C_EDIT_FILE | GMT_C_READ_FILE);
}
bool CFileMsgManager::CanProcess()
{
	if (CMsgProcessBase::CanProcess())
	{
		const CMsgEditDB &editInfo = *(const CMsgEditDB *)pMsg;
		return pMsg->m_iOperatorType == GOT_LOCK || pMsg->m_iOperatorType == GOT_UNLOCK;
	}
	return false;
}
void CFileMsgManager::AddProcessor(CFileMsgManager *pManager, CMsgProcessBaseFile *pProcessor, size_t iMask)
{
	pProcessor->m_iMask = iMask;
	//pProcessor->m_pNext = m_pNext;
	pProcessor->m_pManager = pManager;
	CMsgProcessBase *pLast = m_pNext;
	while(pLast && pLast->m_pNext)
	{
		pLast = pLast->m_pNext;
	}
	pLast ? pLast->m_pNext = pProcessor : m_pNext = pProcessor;
}

int  CFileMsgManager::GetFileIndex(const std::string &strFile)
{
	static int s_iFilerIndex = 1;
	if (int iID = FindFileIndex(strFile))
		return iID;
	map_FileID[strFile] = s_iFilerIndex;
	return s_iFilerIndex++;
}
int  CFileMsgManager::FindFileIndex(const std::string &strFile)
{
	typedef std::map<std::string, int>::iterator iter;
	iter it = map_FileID.find(strFile);
	if (it != map_FileID.end())
		return it->second;
	return 0;
}

CProjectMsgManager::CProjectMsgManager()
{
	m_iMask = 0xFFFFF;
	AddProcessor(new CDBMsgManager(), GMT_C_EDIT_DB | GMT_C_ADD_DB | GMT_C_DELETE_DB | GMT_C_CLOSE_PROJECT_DB);
	AddProcessor(new CFileMsgManager(), GMT_C_EDIT_FILE | GMT_C_READ_FILE);
}
bool CProjectMsgManager::CanProcess()
{
	return true;
}

void CProjectMsgManager::AddProcessor(CMsgProcessBase *pProcessor, size_t iMask)
{
	pProcessor->m_iMask = iMask;
	CMsgProcessBase *pLast = m_pNext;
	while(pLast && pLast->m_pNext)
	{
		pLast = pLast->m_pNext;
	}
	pLast ? pLast->m_pNext = pProcessor : m_pNext = pProcessor;
}