#include "jsonAPI.h"

#ifdef _DEBUG
#pragma comment(lib, "../../../../jsoncpp/build/vs71/debug/lib_json/json_vc71_libmtd.lib")
#else
#pragma comment(lib, "../../../../jsoncpp/build/vs71/release/lib_json/json_vc71_libmt.lib")
#endif

#define NODE_TYPE "MSGType"

Json::Value JSON_GetRootNode()
{
	return Json::Value();
}
std::string JSON_ToString(const Json::Value &Node)
{
	Json::FastWriter writer;
	return writer.write(Node);
}
bool JSON_Prase(const std::string &str, Json::Value &Node)
{
	Json::Reader reader;
	return reader.parse(str.c_str(), Node);
}
int JSON_GetNodeType(const Json::Value &Node)
{
	int iValue;
	return JSON_GetAttributeValue(Node, iValue, NODE_TYPE) ? iValue : 0;
}
void JSON_SetNodeType(Json::Value &Node, int iVal)
{
	JSON_SetAttributeValue(Node, iVal, NODE_TYPE);
}
bool JSON_Type2Type(const Json::Value &subNode, int &value)
{
	if (Json::intValue == subNode.type())
	{
		value = subNode.asInt();
		return true;
	}
	ASSERT_C(false);
	return false;
}

bool JSON_Type2Type(const Json::Value &subNode, std::string &value)
{
	if (Json::stringValue == subNode.type())
	{
		value = subNode.asString();
		return true;
	}
	ASSERT_C(false);
	return false;
}