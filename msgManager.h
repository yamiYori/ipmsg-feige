#ifndef _MSGMANAGER_H_
#define _MSGMANAGER_H_

#include<sys/types.h>
#include<list>
#include<cstring>
#include"definations.h"
#include"usermanager.h"
#include<cstdio>
#include<string>

using std::list;
using std::string;
class msg_t{
public:
    unsigned int ip;
    string msg;
};

class MsgManager{
private:
    MsgManager()=default;
    static MsgManager* instance;
    list<msg_t> mlist;
public:
    static MsgManager* getInstance();
    int maxlength=MSGLENGTH;
    int PopMsg(msg_t& dst);
    int PushMsg(msg_t& src);
    int size();
};

#endif
