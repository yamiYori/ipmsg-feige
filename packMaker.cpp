#include"packMaker.h"
#include"usermanager.h"
#include"chaos.h"
#include"definations.h"
#include"cstring"


static Chaos* chaos=0;
static UserManager* users=0;

static bool needupdate=true;
static int bufusage[PACKRINGLEN]={0};
static char buff[PACKRINGLEN][BUFFLEN]={0};
static int buflength[PACKRINGLEN]={0};
static int next=0;


int mkHeartPack(char dst[],int magic){
	if(chaos==0){
		chaos=Chaos::getInstance();
		users=UserManager::getInstance();
	}
	int cur=next;
	int len=0;
	next++;
	next=next<PACKRINGLEN?next:0;
	len=mkHeader(buff[cur],magic);
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.usrname)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.grpname)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.macaddr)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.space01)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.phone)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.mail)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.printer)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.space02)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.ipaddr)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.space03)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.icon)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.netstatus)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.sign)+1;
	len+=sprintf(buff[cur]+len,"%s",users->myinfo.computername);
	buflength[cur]=len;
	bufusage[cur]=magic;
	memcpy(dst,buff[cur],buflength[cur]);
	return buflength[cur];
}

int mkHeader(char dst[],int magic,unsigned* rtnPackNum){
	if(chaos==0){
		chaos=Chaos::getInstance();
		users=UserManager::getInstance();
	}
	unsigned packNum=(chaos->packNum)++;
	int len=sprintf(dst,"%s:%u:%s:%s:%u:",vermagic,
			packNum,users->myinfo.sysusrname,
			users->myinfo.computername,magic);
	if(rtnPackNum!=NULL){
		*rtnPackNum=packNum;
	}
	return len;
}

