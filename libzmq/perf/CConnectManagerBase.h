#pragma once
#include <string>

enum SQL_CMD_TYPE{
	SCT_NONE,
	SCT_QUERY,
	SCT_DML,
	SCT_DML_BIN
};
class CConnectManagerBase
{
public:
	CConnectManagerBase();
	~CConnectManagerBase();
};

std::wstring Utf82Unicode(const std::string& utf8string) ;
std::string WideByte2Acsi(const std::wstring& wstrcode);
std::string UTF_82ASCII(const std::string& strUtf8Code);
std::wstring Acsi2WideByte(const std::string& strascii);
std::string ASCII2UTF_8(const std::string& strAsciiCode);
