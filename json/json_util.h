#pragma once

#include <string>
#include <sstream>
#include "json.h"
#include "../utils/utils.h"

using namespace std;
using namespace Json;

inline string getJsonStr(Value &json)
{
	//StyledWriter writer;  
    FastWriter writer;
	return writer.write(json);  
}
inline Value parseJsonStr(const string &str) {
	Reader reader;
	Value json(arrayValue);
	bool succ = reader.parse(str, json);
	if (!succ) json.clear();
	return json;
}


inline int check_bool_member(Value &json, const char *field)
{
    if (json.isMember(field) && json[field].isBool()) {
        return 1;
    }
    return 0;
}
inline int get_bool_member(Value &json, const char *field, bool &b)
{
    if (json.isMember(field) && json[field].isBool()) {
        b = json[field].asBool();
        return 1;
    }
    return 0;
}
inline int get_bool_member_as_int(Value &json, const char *field, int &i)
{
    if (json.isMember(field) && json[field].isBool()) {
        i = (int)json[field].asBool();
        return 1;
    }
    return 0;
}
inline int check_int_member(Value &json, const char *field)
{
    if (json.isMember(field) && json[field].isInt()) {
        return 1;
    }
    return 0;
}
inline int get_int_member(Value &json, const char *field, int &i)
{
    if (json.isMember(field) && json[field].isInt()) {
        i = json[field].asInt();
        return 1;
    }
    return 0;
}
inline int get_int_member_as_string(Value &json, const char *field, string &str)
{
    int i;
    if (!get_int_member(json, field, i)) {
        return 0;
    }
    str = hl_ltostring(i);
    return 1;
}
inline int get_longlong_member(Value &json, const char *field, long long &l)
{
    if (json.isMember(field) && json[field].isInt()) {
        l = json[field].asInt();
        return 1;
    }
    return 0;
}

inline int check_str_member(Value &json, const char *field)
{
    if (json.isMember(field) && json[field].isString()) {
        return 1;
    }
    return 0;
}
inline int get_str_member(Value &json, const char *field, string &str)
{
    if (json.isMember(field) && json[field].isString()) {
        str = json[field].asString();
        return 1;
    }
    return 0;
}
inline int get_str_member_as_longlong(Value &json, const char *field, long long &l)
{
    string str;
    if (!get_str_member(json, field, str)) {
        return 0;
    }
    int conv_succ = 0;
    l = hl_atol(str.c_str(), &conv_succ);
    return conv_succ;
}
inline int get_str_member_as_int(Value &json, const char *field, int &i)
{
    long long l = 0;
    if (!get_str_member_as_longlong(json, field, l)) {
        return 0;
    }
    i = (int)l;
    return 1;
}
inline int get_str_member_as_double(Value &json, const char *field, double &d)
{
    string str;
    if (!get_str_member(json, field, str)) {
        return 0;
    }
    int conv_succ = 0;
    d = hl_atod(str.c_str(), &conv_succ);
    return conv_succ;
}
inline int check_double_member(Value &json, const char *field)
{
    if (json.isMember(field) && (json[field].isDouble() || json[field].isInt())) {
        return 1;
    }
    return 0;
}
inline int get_double_member(Value &json, const char *field, double &d)
{
    if (!json.isMember(field)) {
        return 0;
    }
    if (json[field].isDouble()) {
        d = json[field].asDouble();
    }
    else if (json[field].isInt()) {
        d = (double)json[field].asInt();
    }
    else {
        return 0;
    }
    return 1;
}
inline int check_obj_member(Value &json, const char *field)
{
    if (json.isMember(field) && json[field].isObject()) {
        return 1;
    }
    return 0;
}
inline int check_arr_member(Value &json, const char *field)
{
    if (json.isMember(field) && json[field].isArray()) {
        return 1;
    }
    return 0;
}

