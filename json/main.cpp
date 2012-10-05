#include "json_util.h"

#include <iostream>
using namespace std;

inline Value parseJson(const string &str) {
    Reader reader;
    Value json(arrayValue);
    try{
        bool succ = reader.parse(str, json);
        if (!succ) json.clear();
    }
    catch(...) {
        //TODO
    }
    return json;
}

int main(int argc, char** argv)
{
    //string str("GET / HTTP/1.1\r\n Host: app12345-54321.qzoneapp.com:8002\r\n\r\n");
    string str("[1,2,3]\n");
    Value json = parseJson(str);
    if (json.size() > 0 && json.isObject() && json.isMember("cmd")) {
        cout<<"parse json error"<<endl;
    }
    else {
        cout<<json<<endl;
    }
    //cout<<json.size()<<endl;
    //cout<<json.isMember("cmd")<<endl;
    return 0;
}


