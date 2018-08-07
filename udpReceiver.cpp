#include"udpReceiver.h"
#include"usermanager.h"
#include"msgManager.h"
#include"fileManager.h"
#include"definations.h"
#include"chaos.h"
#include"include/ipmsg.h"
#include"fixedmath.h"
#include"packMaker.h"
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<cstdio>
#include<unistd.h>
#include<cstring>
#include<cstdlib>

static Chaos* chaos=0;
static UserManager* users=0;
static MsgManager* msgs=0;
static FileManager* filels=0;

static char ver[40]={0};
static char raw_pkgnum[30];
static unsigned pkgnum;
static char sysusrname[100]={0};
static char computername[100]={0};
static char raw_command[30];
static unsigned command;
static int optstart;
static user_t tmpusr;
static char *(dest[15])={
	tmpusr.usrname,
	tmpusr.grpname,
	tmpusr.macaddr,
	tmpusr.space01,
	tmpusr.phone,
	tmpusr.mail,
	tmpusr.printer,
	tmpusr.space02,
	tmpusr.ipaddr,
	tmpusr.space03,
	tmpusr.icon,
	tmpusr.netstatus,
	tmpusr.sign,
};
static bool needreply=false;

void* thd_udpReceiver(void*){
	//init..
	chaos=Chaos::getInstance();
	if(unlikely(chaos==NULL)){
		printf("\e[31mFatel error:\e[0mChaos not exist.\n");
		dpf("%s,%d,%p,%d\n",__FILE__,__LINE__,chaos,errno);
		exit(1);
	}
	users=UserManager::getInstance();
	if(unlikely(users==NULL)){
		printf("\e[31mFatel error:\e[0mUserManager not exist.\n");
		dpf("%s,%d,%p,%d\n",__FILE__,__LINE__,users,errno);
		exit(1);
	}
	msgs=MsgManager::getInstance();
	if(unlikely(msgs==NULL)){
		printf("\e[31mFatel error:\e[0mMsgManager not exist.\n");
		dpf("%s,%d,%p,%d\n",__FILE__,__LINE__,msgs,errno);
	}
	filels=FileManager::getInstance();
	if(unlikely(filels==NULL)){
		printf("\e[31mFatel error:\e[0mFileManager not exist.\n");
		dpf("%s,%d,%p,%d\n",__FILE__,__LINE__,filels,errno);
	}

	struct sockaddr_in s_addr;
	struct sockaddr_in c_addr;
	int sock;
	socklen_t addr_len;
	int len;
	char buff[BUFFLEN];
	sock=socket(AF_INET,SOCK_DGRAM,0);
	int opt=1;
	setsockopt(sock,SOL_SOCKET,SO_BROADCAST,(char*)&opt,sizeof(opt));

	if(unlikely(sock==-1)){
		printf("\e[31mFatel error:\e[0mUDP init error.\n");
		dpf("%s,%d,%d,%d\n",__FILE__,__LINE__,sock,errno);
		exit(1);
	}
	memset(&s_addr,0,sizeof(struct sockaddr_in));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);
	s_addr.sin_addr.s_addr=INADDR_ANY;

	int funcAns = bind(sock,(struct sockaddr*)&s_addr,sizeof(s_addr));
	if(unlikely(funcAns==-1)){
		printf("\e[31mFatel error:\e[0mUDP init error.\n");
		dpf("%s,%d,%d,%d\n",__FILE__,__LINE__,funcAns,errno);
		exit(1);
	}
	addr_len=sizeof(c_addr);
	chaos->udpsocket=sock;
	chaos->udpListen=true;

	while(chaos->programOK){
		len=recvfrom(sock,buff,sizeof(buff)-1,0,(struct sockaddr*)&c_addr,&addr_len);
		if(unlikely(len<0)){
			dpf("recvfrom error,%d\n",errno);
			sleep(1);
			continue;
		}
		buff[len]=0;
		dpf("%s\n",buff);
		ver[0]=raw_pkgnum[0]=sysusrname[0]=computername[0]=raw_command[0]=0;
		sscanf(buff,"%[0-9a-zA-Z.]:%[0-9]:%[^:]:%[^:]:%[0-9]:%n",
			   ver,raw_pkgnum,sysusrname,computername,raw_command,&optstart);
		if((ver[0]==0)||(raw_pkgnum[0]==0)||(sysusrname[0]==0)||
				(computername[0]==0)||(raw_command[0]==0)){
			dpf("\e[33munpack error:not a pack\e[0m\n");
			continue;
		}
		command=u32atoi(raw_command);
		dpf("mode:%lx\n",GET_MODE(command));
		switch(GET_MODE(command)){
			case IPMSG_BR_ENTRY:
				needreply=true;
			case IPMSG_ANSENTRY:{
				//USR LIST UPDATE
				tmpusr.ip.sin_addr.s_addr=c_addr.sin_addr.s_addr;
				strcpy(tmpusr.sysusrname,sysusrname);
				strcpy(tmpusr.computername,computername);
				for(int i=optstart,cnt=0,sz=0;i<=len;i++){
					if(buff[i])dest[cnt][sz++]=buff[i];
					else{
						dest[cnt][sz++]=0;
						cnt++;
						if(cnt>12)break;
						sz=0;
					}
				}
				tmpusr.healthy=true;
				int ans=users->Adduser(tmpusr);
				if(ans==FUNCSUCCEED){
				}
				if(needreply){
					char buff[BUFFLEN];
					int len=0;
					len=mkHeartPack(buff,IPMSG_ANSENTRY);
					sendto(sock,buff,len,0,(struct sockaddr*)&c_addr,addr_len);
				}
				dpf("ans entry done\n");
				break;
			}
			case IPMSG_BR_EXIT:{
				//USR LIST DELETE
				int ans=users->Deluser(c_addr);
				if(ans==FUNCSUCCEED){
				}
				break;
			}
			case IPMSG_SENDMSG:{
				if((GET_OPT(command)&IPMSG_FILEATTACHOPT)==IPMSG_FILEATTACHOPT){
					recvfile_t tmprecvf;
					char tmpstr[512];
					tmprecvf.tarIp=c_addr.sin_addr.s_addr;
					sscanf(buff+optstart+1,"%u:%[^:]:%x:%u:%u",&(tmprecvf.fileno),tmpstr,
						&(tmprecvf.filesize),&(tmprecvf.time),&(tmprecvf.filetype));
					tmprecvf.filename=tmpstr;
					tmprecvf.fileno;
					filels->Addrecv(tmprecvf);
				}else{
					msg_t tmpmsg;
					tmpmsg.ip=c_addr.sin_addr.s_addr;
					strrchr(buff,'[')[0]=0;
					strrchr(buff,'[')[0]=0;
					tmpmsg.msg=buff+optstart;
					msgs->PushMsg(tmpmsg);
				}

				if(GET_OPT(command)|IPMSG_SENDCHECKOPT){
					char rtnbuff[BUFFLEN];
					int rtnlen=0;
					rtnlen=mkHeader(rtnbuff,IPMSG_RECVMSG);
					rtnlen+=sprintf(rtnbuff+rtnlen,"%s",raw_pkgnum);
					sendto(sock,rtnbuff,rtnlen,0,(struct sockaddr*)&c_addr,addr_len);
				}
				break;
			}
			case IPMSG_EX_CHATREADY:{
				char sendbuff[BUFFLEN];
				int sendlen=0;
				sendlen=mkHeader(sendbuff,IPMSG_RECVMSG);
				sendlen+=sprintf(sendbuff+sendlen,"%s",raw_pkgnum);
				sendto(sock,sendbuff,sendlen,0,(struct sockaddr*)&c_addr,addr_len);
				if(buff[optstart]=='6'){
					sendlen=mkHeader(sendbuff,IPMSG_EX_CHATREADY);
					sendlen+=sprintf(sendbuff+sendlen,"5")+1;
					sendlen+=sprintf(sendbuff+sendlen,"1")+1;
					sendlen+=sprintf(sendbuff+sendlen,"123");
					sendto(sock,sendbuff,sendlen,0,(struct sockaddr*)&c_addr,addr_len);
				}else if(buff[optstart]=='5'){
					user_t tmp;
					int findans=users->Finduser(tmp,c_addr);
					if(findans==FUNCSUCCEED){
						sendlen=mkHeader(sendbuff,IPMSG_EX_CHATCHECKMAC);
						sendlen+=sprintf(sendbuff+sendlen,"%s",tmp.macaddr);
						sendto(sock,sendbuff,sendlen,0,(struct sockaddr*)&c_addr,addr_len);
					}
				}
				break;
			}
			case IPMSG_EX_CHATCHECKMAC:{
				char sendbuff[BUFFLEN];
				int sendlen=0;
				sendlen=mkHeader(sendbuff,IPMSG_RECVMSG);
				sendlen+=sprintf(sendbuff+sendlen,"%s",raw_pkgnum);
				sendto(sock,sendbuff,sendlen,0,(struct sockaddr*)&c_addr,addr_len);
				break;
			}
			case IPMSG_EX_REFUSEFILE:{
				unsigned tmpfno;
				int optsplit=0;
				sscanf(buff+optstart+optsplit,"%x%n",&tmpfno,&optsplit);
				optsplit++;
				sscanf(buff+optstart+optsplit,"%x",&tmpfno);
				printf("%d",tmpfno);
				filels->Delsend(tmpfno,c_addr.sin_addr.s_addr);
				break;
			}
			case IPMSG_EX_CANCELFILE:{
				unsigned tmpfno;
				int optsplit=0;
				sscanf(buff+optstart+optsplit,"%x%n",&tmpfno,&optsplit);
				optsplit++;
				sscanf(buff+optstart+optsplit,"%x",&tmpfno);
				filels->Delrecv(tmpfno,c_addr.sin_addr.s_addr);
				break;
			}
			case IPMSG_EX_PREGETFILE:{
				unsigned sendlen;
				char sendbuff[BUFFLEN];
				sendlen=mkHeader(sendbuff,IPMSG_SENDMSG);
				sendlen+=sprintf(sendbuff+sendlen,"Unsupported function!")+1;
				sendlen+=sprintf(sendbuff+sendlen,"%s",fontmagic);
				sendto(sock,sendbuff,sendlen,0,(struct sockaddr*)&c_addr,addr_len);
				break;
			}
			default:
				dpf("\e[36mUnset command:%X\e[0m\n",command);
			break;
		}
		needreply=false;

	}

}
