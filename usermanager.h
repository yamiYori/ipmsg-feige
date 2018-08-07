#ifndef __USERMANAGER_H_
#define __USERMANAGER_H_
#include<sys/types.h>
#include<sys/socket.h>
#include<list>
#include<vector>
#include<cstring>
#include"definations.h"
#include<netinet/in.h>
#include<cstdio>
using std::list;
class user_t{
public:
//id
    struct sockaddr_in ip;
//information
	char sysusrname[100];
	char computername[100];
	char usrname[100];
	char grpname[100];
	char macaddr[20];
	char space01[10];//
	char phone[20];
	char mail[100];
	char printer[200];
	char space02[10];//
	char ipaddr[20];
	char space03[10];//
	char icon[10];
	char netstatus[10];
	char sign[1000];
	bool healthy;
	user_t()=default;
	user_t(const struct sockaddr_in sai,const char* sname,
	const char* cname,const char*str){
		ip=sai;
		strcpy(sysusrname,sname);
		strcpy(computername,cname);
		sscanf(str,"%s%s%s%s%s%s%s%s%s%s%s%s%s",
				usrname,grpname,macaddr,space01,phone,mail,printer,
				space02,ipaddr,space03,icon,netstatus,sign);
	}
	user_t(const user_t& src);
	user_t& operator=(const user_t& src);
};

class UserManager{
private:
	UserManager()=default;
	static UserManager* instance;
	list<user_t> ulist;

public:
	user_t myinfo;
	static UserManager* getInstance();

	int	Adduser(user_t& tar);
	int Deluser(struct sockaddr_in tar);
	int Finduser(user_t& tar,const struct sockaddr_in src);
	int Finduser(user_t& tar,unsigned int src);
	int GetUserCopy(list<user_t>& src);
	int UserIllness(void);
};

#endif

