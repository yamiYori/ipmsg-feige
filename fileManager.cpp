#include"fileManager.h"
#include<pthread.h>
#include <atomic>
using namespace std;

static pthread_mutex_t recvDelLock;
static atomic<bool> recvDelPrelock(false);
static atomic<int> recvUseCount(0);

static pthread_mutex_t sendDelLock;
static atomic<bool>sendDelPrelock(false);
static atomic<int>sendUseCount(0);

const char *(filetype_str[])={
						 "<Unknown>","<CommonFile>","<Directory>",
						 "<RtnParent>","<SoftLink>","<CharDev>",
						 "<BlockDev>","<FIFO>","<Unknown>",
						 "<Unknown>","<ResFork>"};

FileManager* FileManager::instance=new FileManager();
FileManager* FileManager::getInstance(){
    static bool flgfirst=true;
    if(flgfirst){
        pthread_mutex_init(&recvDelLock,NULL);
        pthread_mutex_init(&sendDelLock,NULL);
    }
    return instance;
}


int FileManager::Addrecv(recvfile_t& tar){
    bool correctTime=false;
    while(correctTime==false){
        while(recvDelPrelock==true);
        recvUseCount++;
        if(recvDelPrelock==true){
            recvUseCount--;
        }else{
            correctTime=true;
        }
    }
    auto i=recvls.begin();
    auto z=recvls.end();
    bool nofound=true;
    for(;i!=z;i++){
        if((i->fileno==tar.fileno)&&(i->tarIp==tar.tarIp)){
            nofound=false;
            break;
        }
    }
    if(nofound){
        recvUseCount--;
        recvls.push_back(tar);
        return FUNCSUCCEED;
    }else{
        *i=tar;
        recvUseCount--;
        return FUNCFAILED;
    }
}
int FileManager::Delrecv(unsigned tarno,unsigned tarIp){
    pthread_mutex_lock(&recvDelLock);
    recvDelPrelock=true;
    while(recvUseCount!=0);
    auto i=recvls.begin();
    auto z=recvls.end();
    bool nofound=true;
    for(;i!=z;i++){
        if((i->fileno==tarno)&&(i->tarIp==tarIp)){
            nofound=false;
            break;
        }
    }
    if(nofound==false){
        i=recvls.erase(i);
    }

    recvDelPrelock=false;
    pthread_mutex_unlock(&recvDelLock);
    return nofound==false?FUNCSUCCEED:FUNCFAILED;
}
int FileManager::Findrecv(unsigned tarno,unsigned tarIp,recvfile_t& tar){
    bool correctTime=false;
    while(correctTime==false){
        while(recvDelPrelock==true);
        recvUseCount++;
        if(recvDelPrelock==true){
            recvUseCount--;
        }else{
            correctTime=true;
        }
    }
    auto i=recvls.begin();
    auto z=recvls.end();
    bool nofound=true;
    for(;i!=z;i++){
        if((i->fileno==tarno)&&(i->tarIp==tarIp)){
            nofound=false;
            break;
        }
    }
    if(nofound){
        recvUseCount--;
        return FUNCFAILED;
    }else{
        tar=*i;
        recvUseCount--;
        return FUNCSUCCEED;
    }
}
int FileManager::GetrecvCopy(list<recvfile_t>& src){
    bool correctTime=false;
    while(correctTime==false){
        while(recvDelPrelock==true);
        recvUseCount++;
        if(recvDelPrelock==true){
            recvUseCount--;
        }else{
            correctTime=true;
        }
    }
    std::copy(recvls.begin(),recvls.end(),std::back_inserter(src));
    recvUseCount--;
}

int FileManager::Addsend(sendfile_t& tar){
    bool correctTime=false;
    while(correctTime==false){
        while(sendDelPrelock==true);
        sendUseCount++;
        if(sendDelPrelock==true){
            sendUseCount--;
        }else{
            correctTime=true;
        }
    }
    auto i=sendls.begin();
    auto z=sendls.end();
    bool nofound=true;
    for(;i!=z;i++){
        if((i->fileno==tar.fileno)&&(i->tarIp==tar.tarIp)){
            nofound=false;
            break;
        }
    }
    if(nofound){
        sendUseCount--;
        sendls.push_back(tar);
        return FUNCSUCCEED;
    }else{
        sendUseCount--;
        return FUNCFAILED;
    }
}
int FileManager::Delsend(unsigned tarno,unsigned tarIp){
    pthread_mutex_lock(&sendDelLock);
    sendDelPrelock=true;
    while(sendUseCount!=0);

    auto i=sendls.begin();
    auto z=sendls.end();
    bool nofound=true;
    for(;i!=z;i++){
        if((i->fileno==tarno)&&(i->tarIp==tarIp)){
            nofound=false;
            break;
        }
    }
    if(nofound==false){
        i=sendls.erase(i);
    }

    sendDelPrelock=false;
    pthread_mutex_unlock(&sendDelLock);
    return nofound==false?FUNCSUCCEED:FUNCFAILED;
}
int FileManager::Findsend(unsigned tarno,unsigned tarIp,sendfile_t& tar){
    bool correctTime=false;
    while(correctTime==false){
        while(sendDelPrelock==true);
        sendUseCount++;
        if(sendDelPrelock==true){
            sendUseCount--;
        }else{
            correctTime=true;
        }
    }
    auto i=sendls.begin();
    auto z=sendls.end();
    bool nofound=true;
    for(;i!=z;i++){
        if((i->fileno==tarno)&&(i->tarIp==tarIp)){
            nofound=false;
            break;
        }
    }
    if(nofound){
        sendUseCount--;
        return FUNCFAILED;
    }else{
        tar=*i;
        sendUseCount--;
        return FUNCSUCCEED;
    }
}
int FileManager::GetsendCopy(list<sendfile_t>& src){
    bool correctTime=false;
    while(correctTime==false){
        while(sendDelPrelock==true);
        sendUseCount++;
        if(sendDelPrelock==true){
            sendUseCount--;
        }else{
            correctTime=true;
        }
    }
    std::copy(sendls.begin(),sendls.end(),std::back_inserter(src));
    sendUseCount--;

}
