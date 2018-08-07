#include"msgManager.h"
#include"definations.h"
#include<list>
#include<cstdlib>
#include<cstring>
#include<string>
#include<cstdio>

MsgManager* MsgManager::instance=new MsgManager();
MsgManager* MsgManager::getInstance(){
    static bool flgfirst=true;
    if(flgfirst){
        ;
    }
    return instance;
}

int MsgManager::PopMsg(msg_t& dst){
    if(!mlist.empty()){
        dst=*(mlist.begin());
        mlist.pop_front();
        return FUNCSUCCEED;
    }
    return FUNCFAILED;
}

int MsgManager::PushMsg(msg_t& src){
    mlist.push_back(src);
    if(mlist.size()>maxlength){
        mlist.pop_front();
    }
    return FUNCSUCCEED;
}

int MsgManager::size(){
    return mlist.size();
}