#include"usermanager.h"
#include"definations.h"
#include<netinet/in.h>
#include<list>
#include<pthread.h>
#include<cstdlib>
#include<cstring>

user_t::user_t(const user_t& src){
	(*this)=src;
}
#define simplyCopy(x) x = src.x
#define deepCopy(x) memcpy( x , src.x ,sizeof( x ))
user_t& user_t::operator=(const user_t& src){
	simplyCopy(ip.sin_family);
	simplyCopy(ip.sin_port);
	simplyCopy(ip.sin_addr.s_addr);
	deepCopy(sysusrname);
	deepCopy(computername);
	deepCopy(usrname);
	deepCopy(grpname);
	deepCopy(macaddr);
	deepCopy(space01);
	deepCopy(phone);
	deepCopy(mail);
	deepCopy(printer);
	deepCopy(space02);
	deepCopy(ipaddr);
	deepCopy(space03);
	deepCopy(icon);
	deepCopy(netstatus);
	deepCopy(sign);
	simplyCopy(healthy);
	return(*this);
}
#undef simplyCopy
#undef deepCopy

static pthread_mutex_t userAddLock;
static pthread_mutex_t userScanLock;
static pthread_mutex_t userCopyLock;
UserManager* UserManager::instance = new UserManager();
UserManager* UserManager::getInstance(){
	static bool flgfirst=true;
	if(flgfirst){			//2-step init
		flgfirst=!flgfirst;
		pthread_mutex_init(&userAddLock,NULL);
		pthread_mutex_init(&userScanLock,NULL);
		pthread_mutex_init(&userCopyLock,NULL);
	}
	return instance;
}

#define check_replace(x) do{		\
	if(strcmp(tar.x , i->x)!=0){	\
		fixed=true;					\
		strcpy(i->x , tar.x);		\
	}}while(0)
int UserManager::Adduser(user_t& tar){
	bool nofound=true;
	pthread_mutex_lock(&userAddLock);
	auto i=ulist.begin();
	auto z=ulist.end();
	for(;i!=z;i++){
		if(i->ip.sin_addr.s_addr==tar.ip.sin_addr.s_addr){
			nofound=false;
			break;
		}
	}
	if(nofound){
		pthread_mutex_unlock(&userAddLock);
		ulist.push_back(tar);
		return FUNCSUCCEED;
	}else{
		bool fixed=false;
		check_replace(sysusrname);
		check_replace(computername);
		check_replace(usrname);
		check_replace(grpname);
		check_replace(macaddr);
		check_replace(space01);
		check_replace(phone);
		check_replace(mail);
		check_replace(printer);
		check_replace(space02);
		check_replace(ipaddr);
		check_replace(space03);
		check_replace(icon);
		check_replace(netstatus);
		check_replace(sign);
		i->healthy=true;
		pthread_mutex_unlock(&userAddLock);
		if(fixed){
			return FUNCSUCCEED;
		}else{
			return FUNCFAILED;
		}
	}
}
#undef check_replace

int UserManager::Deluser(struct sockaddr_in tar){
	pthread_mutex_lock(&userAddLock);
	pthread_mutex_lock(&userScanLock);
	pthread_mutex_lock(&userCopyLock);
	bool nofound=true;
	auto i=ulist.begin();
	auto z=ulist.end();
	for(;i!=z;i++){
		if(i->ip.sin_addr.s_addr==tar.sin_addr.s_addr){
			nofound=false;
			break;
		}
	}
	if(nofound==false){
		i=ulist.erase(i);
	}
	pthread_mutex_unlock(&userAddLock);
	pthread_mutex_unlock(&userScanLock);
	pthread_mutex_unlock(&userCopyLock);
	if(nofound==false){
		return FUNCSUCCEED;
	}else{
		return FUNCFAILED;
	}
}

int UserManager::Finduser(user_t &tar,const struct sockaddr_in src){
	return Finduser(tar,src.sin_addr.s_addr);
}

int UserManager::Finduser(user_t&tar ,unsigned int src){
	pthread_mutex_lock(&userScanLock);
	bool nofound=true;
	auto i=ulist.begin();
	auto z=ulist.end();
	for(;i!=z;i++){
		if(i->ip.sin_addr.s_addr==src){
			nofound=false;
			tar=*i;
			break;
		}
	}
	pthread_mutex_unlock(&userScanLock);
	if(nofound==false){
		return FUNCSUCCEED;
	}else{
		return FUNCFAILED;
	}
}

int UserManager::GetUserCopy(list<user_t>&src){
	pthread_mutex_lock(&userCopyLock);
	std::copy(ulist.begin(),ulist.end(),std::back_inserter(src));
	pthread_mutex_unlock(&userCopyLock);
	return FUNCSUCCEED;
}

int UserManager::UserIllness(void){
	pthread_mutex_lock(&userCopyLock);
	auto i=ulist.begin();
	auto z=ulist.end();
	for(;i!=z;i++){
		i->healthy=false;
	}
	pthread_mutex_unlock(&userCopyLock);
	return FUNCSUCCEED;
}