#include "msginterface.h"

//
void buildDBUserJson(Value &json, DBUser &dbuser) {
    json["uid"] = dbuser.user_name;
    json["id"] = dbuser.user_id;
    json["name"] = dbuser.truename;
    json["sex"] = dbuser.sex;
    json["height"] = dbuser.height;
    json["weight"] = dbuser.weight;
    json["birthday"] = dbuser.birthday;
    json["place"] = dbuser.place;
    json["job"] = dbuser.job;
    json["experience"] = dbuser.experience;
    json["headurl"] = dbuser.headurl; 
    json["selfdescription"] = "null";
}

Value buildFriendsJson(vector <DBFriend> &dbfriends) {
    Value friends(arrayValue);
    for (size_t i = 0; i < dbfriends.size(); i++) {
       Value json(objectValue);
       json["type"] = dbfriends[i].type;
       json["state"] = dbfriends[i].dbuser.state;
       buildDBUserJson(json, dbfriends[i].dbuser); 
       friends.append(json);
    }
    return friends;
}

Value buildGroupsJson(vector <DBGroup> &dbgroups) {
    Value groups(arrayValue);
    for (size_t i = 0; i < dbgroups.size(); i++) {
        Value json(objectValue);
        json["name"] = dbgroups[i].name;
        json["notice"] = dbgroups[i].notice;
        json["headurl"] = dbgroups[i].headurl;
        Value memberjson(arrayValue);
        for (size_t j = 0; j < dbgroups[i].members.size(); j++) { 
            Value userjson(objectValue);
            buildDBUserJson(userjson, dbgroups[i].members[j]);
            memberjson.append(userjson); 
        }
        json["members"] = memberjson;
        groups.append(json);
    } 
    return groups; 
}

Value buildTalksJson(vector <DBTalks> &dbtalks) {
    Value talks(arrayValue);
    for (size_t i = 0; i < dbtalks.size(); i++) {
        Value json(objectValue);
        json["name"] = dbtalks[i].name;
        json["notice"] = dbtalks[i].notice;
        json["headurl"] = dbtalks[i].headurl;
        Value memberjson(arrayValue);
        for (size_t j = 0; j < dbtalks[i].members.size(); j++) {
            Value userjson(objectValue);
            buildDBUserJson(userjson, dbtalks[i].members[j]);
            memberjson.append(userjson); 
        }
        json["members"] = memberjson;
        talks.append(json);
    } 
    return talks; 

}

void buildDBInterfaceJson(Value &json, DBInterface &dbinterface) {
    buildDBUserJson(json, dbinterface.dbuser);
    json["friends"] = buildFriendsJson(dbinterface.dbfriends);
    json["groups"] = buildGroupsJson(dbinterface.dbgroups);
    json["talks"] = buildTalksJson(dbinterface.dbtalks); 
}


