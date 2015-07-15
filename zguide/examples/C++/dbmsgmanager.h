#ifndef __DBMSGMANAGER_HPP_INCLUDED__
#define __DBMSGMANAGER_HPP_INCLUDED__

#include "jsonAPI.h"
#include "msgdefine.h"
#include <map>
#include <list>
#include <string>

typedef std::pair<int, int> CDBItemInfo;

class CDBMsgManager;
class CFileMsgManager;
//CMsgProcessBase
class CMsgProcessBase
{
public:
	virtual ~CMsgProcessBase();
	CMsgProcessBase();
	bool Process(size_t iMsgTypeValue, const CMsgBase *pMsgValue);
	bool Process(const CMsgBase &msgValue);
	virtual bool DoProcess();

protected:
	bool ProcessDispatch();
	bool ProcessNext();
	virtual bool CanProcess();
	//virtual bool IsStateConflict(size_t scr, size_t des);	//check state conflict true: conflict
	virtual bool DoProcessLock();
	virtual bool DoProcessUnLock();

public:
	CMsgProcessBase *m_pNext;
	size_t m_iMask;	//GMT_C_EDIT_DB
protected:
	size_t iMsgType;
	const CMsgBase *pMsg;
};
class CMsgProcessBaseDB : public CMsgProcessBase
{
public:
	CMsgProcessBaseDB();
	//virtual bool IsStateConflict(size_t scr, size_t des);	//check state conflict true: conflict

public:
	CDBMsgManager *m_pManager;
};

class CMsgProcessBaseFile : public CMsgProcessBase
{
public:
	CMsgProcessBaseFile();
	//virtual bool IsStateConflict(size_t scr, size_t des);	//check state conflict true: conflict

public:
	CFileMsgManager *m_pManager;
};
class CMsgManagerBase : public CMsgProcessBase
{
public:
	CMsgManagerBase();
	//virtual bool CanProcess();
	//void AddProcessor(CDBMsgManager *pManager, CMsgProcessBaseDB *pProcessor, size_t iMask);
public:
	int  GetUserIndex(const std::string &strUUID);
	int  FindUserIndex(const std::string &strUUID);

public:
	//std::map<CDBItemInfo, std::map<int, size_t> > map_UserStates;	//(DataType, DataID) : (UserIndex, DataState)
	//typedef std::map<CDBItemInfo, std::map<int, size_t> >::iterator iter;
	//typedef std::map<int, size_t>::iterator iterUser;
	//std::map<int, std::map<int, size_t> > map_TableStates;	//(TableType) : (UserIndex, DataState)

	std::map<std::string, int> map_UserID;//UserUUIDString : UserIndex
};

class CDBMsgManager : public CMsgManagerBase
{
public:
	CDBMsgManager();
	virtual bool CanProcess();
	void AddProcessor(CDBMsgManager *pManager, CMsgProcessBaseDB *pProcessor, size_t iMask);

public:
	std::map<CDBItemInfo, std::map<int, size_t> > map_UserStates;	//(DataType, DataID) : (UserIndex, DataState)
	typedef std::map<CDBItemInfo, std::map<int, size_t> >::iterator iter;
	std::map<int, std::map<int, size_t> > map_TableStates;	//(TableType) : (UserIndex, DataState)
};

//CFileMsgManager
class CFileMsgManager : public CMsgManagerBase
{
public:
	CFileMsgManager();
	virtual bool CanProcess();
	void AddProcessor(CFileMsgManager *pManager, CMsgProcessBaseFile *pProcessor, size_t iMask);
	int  GetFileIndex(const std::string &strFile);
	int  FindFileIndex(const std::string &strFile);
public:
	std::map<int, std::map<int, size_t> > map_UserStates;	//FileName : (UserIndex, DataState)
	typedef std::map<int, std::map<int, size_t> >::iterator iter;
	std::map<std::string, int> map_FileID;//strFileName : FileIndex

};

//CProjectMsgManager
class CProjectMsgManager : public CMsgProcessBase
{
public:
	CProjectMsgManager();
	virtual bool CanProcess();
	void AddProcessor(CMsgProcessBase *pProcessor, size_t iMask);

	GDS_PROJECT_STATE m_ProjectState;
};

#endif
