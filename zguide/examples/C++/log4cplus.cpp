#include "log4cplus.h"
#include <log4cplus/consoleappender.h>
#include <log4cplus/loggingmacros.h>


#ifdef _DEBUG
#pragma comment(lib, "../../../../log4cplus/msvc10/Win32/bin.Debug/log4cplusD.lib")
#else
#pragma comment(lib, "../../../../log4cplus/msvc10/Win32/bin.release/log4cplus.lib")
#endif
using namespace log4cplus;

void InitLog4Plus()
{
	log4cplus::initialize ();
	INIT_LOG("C:\\zeromq\\zguide\\builds\\msvc\\Debug\\log4cplus.properties");  
	SharedAppenderPtr append_1(new ConsoleAppender());
	append_1->setName(LOG4CPLUS_TEXT("First"));
	Logger::getRoot().addAppender(append_1);
	SET_LOG_LEVEL( log4cplus::ALL_LOG_LEVEL); 
	//LOG_DEBUG("debug1");  
	//LOG_ERROR("error2");  
	//LOG_INFO("info3");  
	//LOG_TRACE("trace"); 
}
