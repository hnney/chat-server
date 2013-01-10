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
    json["invited"] = dbuser.invited;
    json["state"] = dbuser.state;
    
    json["membertype"] = dbuser.subtype;
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
        json["uid"] = dbgroups[i].name;
        json["notice"] = dbgroups[i].notice;
        json["headurl"] = dbgroups[i].headurl;
        json["id"] = dbgroups[i].group_id;
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
        json["id"] = dbtalks[i].talk_id;
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

void parse_friends(Value &json, vector <DBFriend> &dbfriends) {
    if (json.size() > 0) {
        dbfriends.resize(json.size());
    }
    for (size_t i = 0; i < json.size(); i++) {
        get_int_member(json[i], "id", dbfriends[i].dbuser.user_id);    
        //TODO
    }
}

void parse_groups(Value &json, vector <DBGroup> &dbgroups) {
    if (json.size() > 0) {
        dbgroups.resize(json.size());
    }
    for (size_t i = 0; i < json.size(); i++) {
        get_int_member(json[i], "id", dbgroups[i].group_id);
        //TODO
        //member
        if (check_arr_member(json[i], "members")) {
            size_t s = json[i]["members"].size();
            if (s > 0) {
                dbgroups[i].members.resize(s);
            }
            for (size_t j = 0; j < s; j++) {
                get_int_member(json[i]["members"][j], "id", dbgroups[i].members[j].user_id);    
            }
        } 
    }
}
void parse_talks(Value &json, vector <DBTalks> &dbtalks) {
    if (json.size() > 0) {
        dbtalks.resize(json.size());
    }
    for (size_t i = 0; i < json.size(); i++) {
        get_int_member(json[i], "id", dbtalks[i].talk_id);
        //TODO
        //member
        if (check_arr_member(json[i], "members")) {
            size_t s = json[i]["members"].size();
            if (s > 0) {
                dbtalks[i].members.resize(s);
            }
            for (size_t j = 0; j < json[i]["members"].size(); j++) {
                get_int_member(json[i]["members"][j], "id", dbtalks[i].members[j].user_id);    
            }
        } 
    }
}

int parse_dbinterface(Value &json, DBInterface &dbinterface) {
    int ret = -1;
    if (!json.isObject()) {
        return ret;
    }
    if (get_int_member(json, "id", dbinterface.dbuser.user_id)) {
        ret = 0;
    }
    get_int_member(json, "invited", dbinterface.dbuser.invited);
    if (check_arr_member(json, "friends")) {
        parse_friends(json["friends"], dbinterface.dbfriends); 
    }
    if (check_arr_member(json, "groups")) {
        parse_groups(json["groups"], dbinterface.dbgroups);
    }
    if (check_arr_member(json, "talks")) {
        parse_talks(json["talks"], dbinterface.dbtalks);
    }
    return ret;
}


