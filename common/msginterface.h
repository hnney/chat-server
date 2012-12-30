#ifndef __MSG_INTERFACE_H__
#define __MSG_INTERFACE_H__
#include "../common/dbstruct.h"
#include "../json/json_util.h"

void buildDBUserJson(Value &json, DBUser &dbuser);
Value buildFriendsJson(vector <DBFriend> &dbfriends);
Value buildGroupsJson(vector <DBGroup> &dbgroups);
Value buildTalksJson(vector <DBTalks> &dbtalks);
void buildDBInterfaceJson(Value &json, DBInterface &dbinterface);



#endif //

