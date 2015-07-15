#ifndef __LOG4CPLUS_HPP_INCLUDED__
#define __LOG4CPLUS_HPP_INCLUDED__

#ifdef _DEBUG
#define ASSERT_C(_Expression) assert(_Expression)
#else
#define ASSERT_C(_Expression)
#endif
#include <assert.h>

#include <log4cplus/logger.h> 
#include <log4cplus/configurator.h> 
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/loggingmacros.h>

using namespace log4cplus;
using namespace log4cplus::helpers;

void InitLog4Plus();
const LogLevel CRITICAL_LOG_LEVEL = 45000;

//ʹ����־���ó�ʼ��log��־
#define INIT_LOG(filePath) log4cplus::PropertyConfigurator::doConfigure(filePath)

//������־����
#define SET_LOG_LEVEL(level) log4cplus::Logger::getRoot().setLogLevel(level)

//������־���
#define LOG_TRACE(logs) LOG4CPLUS_TRACE(log4cplus::Logger::getRoot(), logs)
#define LOG_DEBUG(event) LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), event)
#define LOG_INFO(logs) LOG4CPLUS_INFO(log4cplus::Logger::getRoot(), logs)
#define LOG_ERROR(logs) LOG4CPLUS_ERROR(log4cplus::Logger::getRoot(), logs)


#define LOG4CPLUS_CRITICAL(logger, logEvent) \
	if(logger.isEnabledFor(CRITICAL_LOG_LEVEL)) { \
	log4cplus::tostringstream _log4cplus_buf; \
	_log4cplus_buf << logEvent; \
	logger.forcedLog(CRITICAL_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
	}

#endif
