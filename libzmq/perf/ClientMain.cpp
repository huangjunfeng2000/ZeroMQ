#include "ClientMain.h"
#define  WIN32_LEAN_AND_MEAN
#include <msgpack.hpp>
#include "CConnectManagerClient.h"

// Hello World client
// Connects REQ socket to tcp://localhost:5555
// Sends "Hello" to server, expects "World" back
//
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace std;
//UTF-8转Unicode  
//std::wstring Utf82Unicode(const std::string& utf8string)  
//{  
//	int widesize = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, NULL, 0);  
//	if (widesize == ERROR_NO_UNICODE_TRANSLATION)  
//	{  
//		throw std::exception("Invalid UTF-8 sequence.");  
//	}  
//	if (widesize == 0)  
//	{  
//		throw std::exception("Error in conversion.");  
//	}  
//
//	std::vector<wchar_t> resultstring(widesize);  
//
//	int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, &resultstring[0], widesize);  
//
//	if (convresult != widesize)  
//	{  
//		throw std::exception("La falla!");  
//	}  
//
//	return std::wstring(&resultstring[0]);  
//}  
//
////unicode 转为 ascii  
//
//string WideByte2Acsi(wstring& wstrcode)  
//{  
//	int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, NULL, 0, NULL, NULL);  
//	if (asciisize == ERROR_NO_UNICODE_TRANSLATION)  
//	{  
//		throw std::exception("Invalid UTF-8 sequence.");  
//	}  
//	if (asciisize == 0)  
//	{  
//		throw std::exception("Error in conversion.");  
//	}  
//	std::vector<char> resultstring(asciisize);  
//	int convresult =::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, &resultstring[0], asciisize, NULL, NULL);  
//
//	if (convresult != asciisize)  
//	{  
//		throw std::exception("La falla!");  
//	}  
//
//	return std::string(&resultstring[0]);  
//}  
//
////utf-8 转 ascii  
//
//string UTF_82ASCII(string& strUtf8Code)  
//{  
//	string strRet("");  
//
//
//	//先把 utf8 转为 unicode   
//	wstring wstr = Utf82Unicode(strUtf8Code);  
//
//	//最后把 unicode 转为 ascii  
//	strRet = WideByte2Acsi(wstr);  
//
//
//	return strRet;  
//}  
//
/////////////////////////////////////////////////////////////////////////  
//
////ascii 转 Unicode  
//
//wstring Acsi2WideByte(string& strascii)  
//{  
//	int widesize = MultiByteToWideChar (CP_ACP, 0, (const char*)strascii.c_str(), -1, NULL, 0);  
//	if (widesize == ERROR_NO_UNICODE_TRANSLATION)  
//	{  
//		throw std::exception("Invalid UTF-8 sequence.");  
//	}  
//	if (widesize == 0)  
//	{  
//		throw std::exception("Error in conversion.");  
//	}  
//	std::vector<wchar_t> resultstring(widesize);  
//	int convresult = MultiByteToWideChar (CP_ACP, 0, (char*)strascii.c_str(), -1, &resultstring[0], widesize);  
//
//	if (convresult != widesize)  
//	{  
//		throw std::exception("La falla!");  
//	}  
//
//	return std::wstring(&resultstring[0]);  
//}  
//
////Unicode 转 Utf8  
//
//std::string Unicode2Utf8(const std::wstring& widestring)  
//{  
//	int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, NULL, 0, NULL, NULL);  
//	if (utf8size == 0)  
//	{  
//		throw std::exception("Error in conversion.");  
//	}  
//
//	std::vector<char> resultstring(utf8size);  
//
//	int convresult = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, &resultstring[0], utf8size, NULL, NULL);  
//
//	if (convresult != utf8size)  
//	{  
//		throw std::exception("La falla!");  
//	}  
//
//	return std::string(&resultstring[0]);  
//}  
//
////ascii 转 Utf8  
//
//string ASCII2UTF_8(string& strAsciiCode)  
//{  
//	string strRet("");  
//
//
//	//先把 ascii 转为 unicode   
//	wstring wstr = Acsi2WideByte(strAsciiCode);  
//
//	//最后把 unicode 转为 utf8  
//
//	strRet = Unicode2Utf8(wstr);  
//
//
//	return strRet;  
//} 

//#include <unistd.h>
#include <conio.h>
int main_req () {
	void *context = zmq_init (1);
	// Socket to talk to server
	printf ("Connecting to hello world server...\n");
	void *requester = zmq_socket (context, ZMQ_REQ);
	zmq_connect (requester, "tcp://localhost:5555");
	int request_nbr;
	for (request_nbr = 0; request_nbr != 1000; request_nbr++) {
		zmq_msg_t request;
		zmq_msg_init_data (&request, "Hello", 6, NULL, NULL);
		printf ("Sending request %d...\n", request_nbr);
		zmq_sendmsg (requester, &request, 0);
		zmq_msg_close (&request);
		zmq_msg_t reply;
		zmq_msg_init (&reply);
		zmq_recvmsg (requester, &reply, 0);
		printf ("Received reply %d: [%s]\n", request_nbr,
			(char *) zmq_msg_data (&reply));
		zmq_msg_close (&reply);
	}
	zmq_close (requester);
	zmq_term (context);
	return 0;
}

int main_req_file_trans () {
	void *context = zmq_init (1);
	// Socket to talk to server
	printf ("Connecting to hello world server...\n");
	void *requester = zmq_socket (context, ZMQ_REQ);
	zmq_connect (requester, "tcp://localhost:5555");
	int request_nbr = 0;
		const char *strFile = "c:\\Column2.bmp";


	for (request_nbr = 0; request_nbr < 400; request_nbr++) 
	{
		int start = GetTickCount();
		char str[30];
		//for (int i=0; i<100; ++i)
		int i=0;
		int iStart = 0;
		int iCount = 3000 + (request_nbr +1 ) * 5000;
		int iWriteCount = 0;
		while(true)
		{
			memset(str, '\0', 30);
			sprintf_s(str, "%d %d ", iStart, iCount);
			iStart += iCount;
			++i;
			zmq_msg_t request;
			zmq_msg_init_data (&request, str, strlen(str), NULL, NULL);
			//printf ("Sending request %s...\n", str);
			zmq_sendmsg (requester, &request, 0);
			zmq_msg_close (&request);
			zmq_msg_t reply;
			zmq_msg_init (&reply);
			zmq_recvmsg (requester, &reply, 0);
			int iReadCount = zmq_msg_size(&reply);

			if (iReadCount==0)
				break;
			//printf ("Received reply %d: [%-4d] \n", i, 
			//	iReadCount/*, (char *) zmq_msg_data (&reply)*/);
			//if (request_nbr == 0)
			//{
				int start1 = GetTickCount();
				void *pVal = zmq_msg_data(&reply);
				if (FILE *fp = fopen(strFile, i==1 ? "wb" : "ab"))
				//if (FILE *fp = fopen(strFile, i>0 ? "wb" : "ab"))
				{
					fseek(fp, 0, SEEK_END);
					fwrite(pVal, iReadCount, 1, fp);
					fclose(fp);
				}
				int stop1 = GetTickCount();
				iWriteCount += stop1 - start1;
			//}

			zmq_msg_close (&reply);
		}
		int stop = GetTickCount();
		printf("time: %d %d total:%d ms write:%d ms\n", i, iCount, stop - start, iWriteCount);
	}
	zmq_close (requester);
	zmq_term (context);

	getch();
	return 0;
}

int main_req_file_trans_pack () {
	void *context = zmq_init (1);
	// Socket to talk to server
	printf ("Connecting to hello world server...\n");
	void *requester = zmq_socket (context, ZMQ_REQ);
	zmq_connect (requester, "tcp://localhost:5555");
	int request_nbr = 0;
	const char *strFile = "c:\\Column2.bmp";


	for (request_nbr = 100; request_nbr < 600; request_nbr++) 
	{
		int start = GetTickCount();
		char str[30];
		//for (int i=0; i<100; ++i)
		int i=0;
		int iStart = 0;
		int iCount = 3000 + (request_nbr +1 ) * 5000;
		int iWriteCount = 0;
		while(true)
		{
			memset(str, '\0', 30);
			sprintf_s(str, "%d %d ", iStart, iCount);
			iStart += iCount;
			++i;
			zmq_msg_t request;
			zmq_msg_init_data (&request, str, strlen(str), NULL, NULL);
			//printf ("Sending request %s...\n", str);
			zmq_sendmsg (requester, &request, 0);
			zmq_msg_close (&request);
			zmq_msg_t reply;
			zmq_msg_init (&reply);
			zmq_recvmsg (requester, &reply, 0);
			int iReadCount = zmq_msg_size(&reply);


			//printf ("Received reply %d: [%-4d] \n", i, 
			//	iReadCount/*, (char *) zmq_msg_data (&reply)*/);
			//if (request_nbr == 0)
			//{
			int start1 = GetTickCount();
			void *pVal = zmq_msg_data(&reply);
			int iSize = zmq_msg_size(&reply);
			std::size_t off = 0;
			msgpack::unpacked msgPack;
			msgpack::unpack(msgPack, (char *)pVal, iSize);
			const msgpack::object& packObj = msgPack.get();
			msgpack::type::ext_ref val2;
			packObj.convert(val2);
			//char *pResStr = packObj.as<char *>();
			int iReadCount2 = val2.size();
			iReadCount = iReadCount2;
			int iValType = val2.type();
			if (iReadCount==0)
				break;
			pVal = (void *)val2.data();//msgPack.buffer();
			if (FILE *fp = fopen(strFile, i==1 ? "wb" : "ab"))
			//if (FILE *fp = fopen(strFile, i>0 ? "wb" : "ab"))
			{
				fseek(fp, 0, SEEK_END);
				fwrite(pVal, iReadCount, 1, fp);
				fclose(fp);
			}
			int stop1 = GetTickCount();
			iWriteCount += stop1 - start1;
			//}

			zmq_msg_close (&reply);
		}
		int stop = GetTickCount();
		printf("time: %d %d total:%d ms write:%d ms\n", i, iCount, stop - start, iWriteCount);
	}
	zmq_close (requester);
	zmq_term (context);

	getch();
	return 0;
}

int main_req_db_trans_pack () {
	void *context = zmq_init (1);
	// Socket to talk to server
	printf ("Connecting to hello world server...\n");
	void *requester = zmq_socket (context, ZMQ_REQ);
	zmq_connect (requester, "tcp://localhost:5555");
	int request_nbr = 0;
	//const char *strFile = "c:\\Column2.bmp";

	enum {sqlCount = 3};
	std::string strQur[sqlCount] = {
		"select * from tbl_wellinfo;",
		"update tbl_wellinfo set wellname=\"A\"",
		"select max(wellid) as maxid from tbl_wellinfo;"
	};
	CConnectManagerClient cmClient(requester);
	bool bExport = false;

	for (int i = 0; i < sqlCount; i++) 
	{
		int start = GetTickCount();

		CMQSqlQuery *pQury = cmClient.ExecQuery(strQur[i]);
		while(pQury && pQury->next())
		{
			for (int j=0; j<pQury->msgInfo.iFieldCount; ++j)
			{
				const msgpack::object &obj = pQury->nextvalue();
				pQury->printval(obj);
			}
		}
		delete pQury;

		//char str[300];
		////for (int i=0; i<100; ++i)
		////int i=0;
		//int iStart = 0;
		////int iCount = 3000 + (request_nbr +1 ) * 5000;
		//int iWriteCount = 0;
		////while(true)
		//{
		//	memset(str, '\0', 300);
		//	std::string strQuery = ASCII2UTF_8(strQur[i]);
		//	sprintf_s(str, "%s", strQuery.c_str());
		//	std::cout <<endl <<  "Query(" << i+1 <<"): " << "(" << strQuery.substr(0, 60) << ")";
		//	//iStart += iCount;
		//	//++i;
		//	
		//	zmq_msg_t request;
		//	zmq_msg_init_data (&request, str, strlen(str)+1, NULL, NULL);
		//	//printf ("Sending request %s...\n", str);
		//	zmq_sendmsg (requester, &request, 0);
		//	zmq_msg_close (&request);
		//	zmq_msg_t reply;
		//	zmq_msg_init (&reply);
		//	zmq_recvmsg (requester, &reply, 0);
		//	//int iReadCount = zmq_msg_size(&reply);

		//	//printf ("Received reply %d: [%-4d] \n", i, 
		//	//	iReadCount/*, (char *) zmq_msg_data (&reply)*/);
		//	//if (request_nbr == 0)
		//	//{
			//int start1 = GetTickCount();
		//	const char *pVal = (const char *)zmq_msg_data(&reply);
		//	size_t iSize = zmq_msg_size(&reply);

		//	std::size_t off = 0;
		//	msgpack::unpacked msg;
		//	msgpack::unpack(msg, pVal, iSize, off);
		//	int iFieldCount = msg.get().as<int>();
		//	//for (int i=0; i<iFieldCount; ++i)
		//	int iFieldIndex = 0;
		//	int iRowIndex = 0;
		//	while(off < iSize)
		//	{
		//		iFieldIndex = iFieldIndex % (iFieldCount);
		//		if (iFieldIndex == 0 && bExport)
		//			printf("\n--------------------------V%-3d--------------------------\n", ++iRowIndex);
		//		msgpack::unpack(msg, pVal, iSize, off);
		//		const msgpack::object& msgObj = msg.get();
		//		if (msgObj.is_nil())
		//		{
		//			if (iFieldIndex != iFieldCount-1)
		//				assert(false);
		//			break;
		//		}			
		//		
		//		switch(msgObj.type)
		//		{
		//		case msgpack::type::BOOLEAN:
		//			{
		//				bool tempVal = msgObj.as<bool>();
		//				if (bExport) 
		//					printf("%d ", tempVal);
		//			}
		//			break;
		//		case msgpack::type::POSITIVE_INTEGER:
		//		case msgpack::type::NEGATIVE_INTEGER:
		//			{
		//				int tempVal = msgObj.as<int>();
		//				if (bExport)
		//					printf("%d ", tempVal);
		//			}
		//			break;
		//		case msgpack::type::FLOAT:
		//			{
		//				float tempVal = msgObj.as<float>();
		//				if (bExport)
		//					printf("%.3f ", tempVal);
		//			}
		//			break;
		//		case msgpack::type::STR:
		//			{
		//				//bool tempVal = msgObj.as<bool>();
		//				int iSize = msgObj.via.str.size;
		//				const char *tempVal = msgObj.via.str.ptr;
		//				std::string str(tempVal, iSize);
		//				str = UTF_82ASCII(str);
		//				if (bExport)
		//					printf("%s ", iSize ? str.c_str() : "NL");
		//			}
		//			break;
		//		case msgpack::type::BIN:
		//			{
		//				const char *tempVal = msgObj.via.bin.ptr;
		//				int iSize = msgObj.via.bin.size;
		//				if (bExport)
		//					printf("BLOB(%d) ", iSize);
		//			}
		//			break;
		//		case msgpack::type::EXT:
		//			{
		//				const char *tempVal = msgObj.via.ext.data();
		//				int iSize = msgObj.via.bin.size;
		//				if (bExport)
		//					printf("EXT(%d) ", "EXT");
		//			}
		//			break;
		//		case msgpack::type::NIL:
		//			assert(false);
		//			break;
		//		default:
		//			assert(false);
		//			break;
		//		}
		//		++iFieldIndex;
		//	}
			//std::size_t off = 0;
			//msgpack::unpacked msgPack;
			//msgpack::unpacker pac; 
			//pac.reserve_buffer(iSize);


			//msgpack::unpack(msgPack, (char *)pVal, iSize);


			//msgpack::type::ext_ref val2;
			//msgPack.get().convert(val2);

			//char *pResStr = packObj.as<char *>();
			//int iReadCount2 = val2.size();
			//iReadCount = iReadCount2;
			//int iValType = val2.type();
			//if (iReadCount==0)
			//	break;
			//pVal = (void *)val2.data();//msgPack.buffer();
			//if (FILE *fp = fopen(strFile, i==1 ? "wb" : "ab"))
			//	//if (FILE *fp = fopen(strFile, i>0 ? "wb" : "ab"))
			//{
			//	fseek(fp, 0, SEEK_END);
			//	fwrite(pVal, iReadCount, 1, fp);
			//	fclose(fp);
			//}
			//int stop1 = GetTickCount();
			//iWriteCount += stop1 - start1;
			//}

			//zmq_msg_close (&reply);
			int stop = GetTickCount();
			printf("\ntotal:%d ms\n", stop - start);
		}
	
	zmq_close (requester);
	zmq_term (context);

	getch();
	return 0;
}