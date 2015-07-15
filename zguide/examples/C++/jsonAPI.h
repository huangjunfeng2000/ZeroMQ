#ifndef __JSONAPI_HPP_INCLUDED__
#define __JSONAPI_HPP_INCLUDED__

#ifdef _DEBUG
#define ASSERT_C(_Expression) assert(_Expression)
#else
#define ASSERT_C(_Expression)
#endif
#include <assert.h>
#include "../../../jsoncpp/include/json/json.h"

Json::Value JSON_GetRootNode();
template<class T> void JSON_SetAttributeValue(Json::Value &node, const T&value, const char *AttrName)
{
	node[AttrName] = value;
}
template<class T> void JSON_SetAttributeValue(Json::Value &node, const T *const &value, const char *AttrName)
{
	node[AttrName] = value;
}
template<class T> bool JSON_Type2Type(const Json::Value &subNode, T &value)
{
	return false;
}
bool JSON_Type2Type(const Json::Value &subNode, int &value);

bool JSON_Type2Type(const Json::Value &subNode, std::string &value);

template<class T> bool JSON_GetAttributeValue(const Json::Value &node, T &value, const char *AttrName)
{
	const Json::Value &subNode = node[AttrName];
	return JSON_Type2Type(subNode, value);
}

std::string JSON_ToString(const Json::Value &Node);
bool JSON_Prase(const std::string &str, Json::Value &Node);
int JSON_GetNodeType(const Json::Value &Node);
void JSON_SetNodeType(Json::Value &Node, int iVal);

#endif
