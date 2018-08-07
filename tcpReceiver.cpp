#include"tcpReceiver.h"
#include"usermanager.h"
#include"fileManager.h"
#include"definations.h"
#include"msgManager.h"
#include"chaos.h"
#include"include/ipmsg.h"
#include"fixedmath.h"
#include"packMaker.h"
#include<pthread.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<cstdio>
#include<fcntl.h>
#include<unistd.h>
#include<cstdlib>
#include<ctime>

using namespace std;

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
static bool needreply=false;

struct threadarg_t{
	int sockfd;
	unsigned clientIp;
};

void* thd_tcpReceiver(void*){
	//init..
	chaos=Chaos::getInstance();
	if(unlikely(chaos==NULL)){
		printf("\e[31mFatel error:\e[0mChaos not exist.\n");
		dpf("%s,%d,%d,%d\n",__FILE__,__LINE__,chaos,errno);
		exit(1);
	}
	users=UserManager::getInstance();
	if(unlikely(users==NULL)){
		printf("\e[31mFatel error:\e[0mUserManager not exist.\n");
		dpf("%s,%d,%d,%d\n",__FILE__,__LINE__,users,errno);
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

	sock = socket(AF_INET, SOCK_STREAM,0);

	int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

	if(unlikely(sock==-1)){
		printf("\e[31mFatel error:\e[0mTCP init error.\n");
		dpf("%s,%d,%d,%d\n",__FILE__,__LINE__,sock,errno);
		exit(1);
	}
	memset(&s_addr,0,sizeof(struct sockaddr_in));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);
	s_addr.sin_addr.s_addr=INADDR_ANY;

	int funcAns = bind(sock,(struct sockaddr*)&s_addr,sizeof(s_addr));
	if(unlikely(funcAns==-1)){
		printf("\e[31mFatel error:\e[0mTCP init error.\n");
		dpf("%s,%d,%d,%d\n",__FILE__,__LINE__,funcAns,errno);
		exit(1);
	}
	int listen_sock=listen(sock,10);
	if(unlikely(listen_sock==-1)){
		printf("\e[31mFatel error:\e[0mTCP init error.\n");
		dpf("%s,%d,%d,%d\n",__FILE__,__LINE__,listen_sock,errno);
	}
	addr_len=sizeof(c_addr);

	chaos->tcpListen=true;
	pthread_t tcpinstance;
	int tcpthdfd;
	while(chaos->programOK){
		int accept_fd=accept(sock,(struct sockaddr*)&c_addr,&addr_len);
		if(accept_fd==-1){
			sleep(1);
			continue;
		}
		threadarg_t *arg=(threadarg_t *)malloc(sizeof(threadarg_t));
		arg->clientIp=c_addr.sin_addr.s_addr;
		arg->sockfd=accept_fd;
		tcpthdfd=pthread_create(&tcpinstance,NULL,tcp_inst,(void *)arg);
		pthread_detach(tcpinstance);
	}

	close(sock);
	return 0;
}

void *tcp_inst(void* arg){
	int sockfd=((threadarg_t*)arg)->sockfd;
	int clientIp=((threadarg_t*)arg)->clientIp;
	free(arg);

	char ver[40]={0};
	char raw_pkgnum[30];
	unsigned pkgnum;
	char sysusrname[100]={0};
	char computername[100]={0};
	char raw_command[30];
	unsigned command;
	int optstart;

	char recvbuff[BUFFLEN];
	int recvlen=recv(sockfd,recvbuff,sizeof(recvbuff),0);
	if(unlikely(recvlen<0)){
		dpf("tcp recv error,%d\n",errno);
		close(sockfd);
		return 0;
	}

	recvbuff[recvlen]=0;
	ver[0]=raw_pkgnum[0]=sysusrname[0]=computername[0]=raw_command[0]=0;

	sscanf(recvbuff,"%[0-9a-zA-Z.]:%[0-9]:%[^:]:%[^:]:%[0-9]:%n",
			   ver,raw_pkgnum,sysusrname,computername,raw_command,&optstart);
	if((ver[0]==0)||(raw_pkgnum[0]==0)||(sysusrname[0]==0)||
			(computername[0]==0)||(raw_command[0]==0)){
		dpf("\e[33mtcppack error:not a pack\e[0m\n");
	}
	command=u32atoi(raw_command);
	if((command&IPMSG_GETFILEDATA)==IPMSG_GETFILEDATA){
		unsigned fileno;
		int optsplit=0;
		sscanf(recvbuff+optstart+optsplit,"%x%n",&fileno,&optsplit);
		optsplit++;
		sscanf(recvbuff+optstart+optsplit,"%x",&fileno);

		if((command==IPMSG_GETFILEDATA)){
			sendfile_t tmpsendfile;
			int findans=filels->Findsend(fileno,clientIp,tmpsendfile);
			if(findans==FUNCSUCCEED){
				char *filebuff=(char *)malloc(tmpsendfile.filesize);

				//FILE* fp=fopen(tmpsendfile.filelocate.c_str(),"rb");
				int fd = open(tmpsendfile.filelocate.c_str(), O_RDONLY);
				//if(fp!=NULL){
				if (fd != -1){
					//fread(filebuff,tmpsendfile.filesize,1,fp);
					int len;
					while ((len = read(fd, filebuff, tmpsendfile.filesize)) > 0){
						//send(sockfd,filebuff,sizeof(tmpsendfile.filesize),0);
						write(sockfd, filebuff, len);
					}
					close(fd);
				}else{
					dpf("\e[33mSendfile failed,file %s open failed:%d.\e[0m\n",tmpsendfile.filelocate.c_str(),errno);
				}
				free(filebuff);
				filels->Delsend(fileno,clientIp);
			}else{
				dpf("\e[33mSendfile failed,file %ld for %lx not exist.\e[0m\n",fileno,clientIp);
			}
		}else if((command&IPMSG_FILE_DIR)==IPMSG_FILE_DIR){
			sendfile_t tmpsendfile;
			int findans=filels->Findsend(fileno,clientIp,tmpsendfile);
			if(findans==FUNCSUCCEED){
				char sendbuff[BUFFLEN];
				int bufheadlen;
				char array_bufheadlen[10];
				sprintf(sendbuff,"0000:%s:%u:%d:14=%lx:16=%lx%n",tmpsendfile.filename,tmpsendfile.filesize,tmpsendfile.filetype,time(NULL),time(NULL),&bufheadlen);
				sprintf(array_bufheadlen,"%04x",bufheadlen);
				memcpy(sendbuff,array_bufheadlen,4);
				send(sockfd,sendbuff,sizeof(sendbuff),0);

				filels->Delsend(fileno,clientIp);
			}else{
				dpf("\e[33mSendfile failed,file %ld for %lx not exist.\e[0m\n",fileno,clientIp);
			}

		}else{
			dpf("\e[33munknown recv command,%d\e[0m\n",command);
		}
	}else{
		dpf("\e[33munknown tcp command,%d\e[0m\n",command);
	}

	close(sockfd);
	return 0;


}
