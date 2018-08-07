#ifndef _FILEMANAGER_H_
#define _FILEMANAGER_H_

#include<sys/types.h>
#include<list>
#include<string>
#include"definations.h"
#include<cstdio>
#include<cstring>

using std::list;
using std::string;
extern const char *(filetype_str[]);

class recvfile_t{
public:
    unsigned tarIp;
    unsigned fileno;
    string filename;
    unsigned filesize;
    unsigned filetype;
    unsigned time;
    unsigned attr;
};
class sendfile_t{
public:
    unsigned tarIp;
    unsigned fileno;
    string filename;
    unsigned filesize;
    string filelocate;
    unsigned filetype;
};
class FileManager{
private:
    FileManager()=default;
    static FileManager* instance;
    list<recvfile_t> recvls;
    list<sendfile_t> sendls;
public:
    static FileManager* getInstance();

    int Addrecv(recvfile_t& tar);
    int Delrecv(unsigned tarno,unsigned tarIp);
    int Findrecv(unsigned tarno,unsigned tarIp,recvfile_t& tar);
    int GetrecvCopy(list<recvfile_t>& src);

    int Addsend(sendfile_t& tar);
    int Delsend(unsigned tarno,unsigned tarIp);
    int Findsend(unsigned tarno,unsigned tarIp,sendfile_t& tar);
    int GetsendCopy(list<sendfile_t>& src);

};
#endif

